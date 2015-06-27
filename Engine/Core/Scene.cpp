#include "Scene.hpp"
#include <list>
#include <string>
#include <sstream>
#include "AudioSourceComponent.hpp"
#include "CameraComponent.hpp"
#include "FileSystem.hpp"
#include "SpriteRendererComponent.hpp"
#include "TextRendererComponent.hpp"
#include "TransformComponent.hpp"
#include "GfxDevice.hpp"
#include "Renderer.hpp"
#include "GameObject.hpp"
#include "System.hpp"

extern ae3d::Renderer renderer;

void ae3d::Scene::Add( GameObject* gameObject )
{
    for (const auto& go : gameObjects)
    {
        if (go == gameObject)
        {
            return;
        }
    }

    if (nextFreeGameObject == gameObjects.size())
    {
        gameObjects.resize( gameObjects.size() + 10 );
    }

    gameObjects[ nextFreeGameObject++ ] = gameObject;

    CameraComponent* camera = gameObject->GetComponent<CameraComponent>();

    if (camera && mainCamera == nullptr)
    {
        mainCamera = gameObject;
    }
}

void ae3d::Scene::Remove( GameObject* gameObject )
{
    for (std::size_t i = 0; i < gameObjects.size(); ++i)
    {
        if (gameObject == gameObjects[ i ])
        {
            gameObjects.erase( std::begin( gameObjects ) + i );
            return;
        }
    }
}

void ae3d::Scene::Render()
{
    if (mainCamera == nullptr || (mainCamera != nullptr && mainCamera->GetComponent<CameraComponent>() == nullptr))
    {
        return;
    }
    
    std::list< GameObject* > rtCameras;
    
    for (auto gameObject : gameObjects)
    {
        if (gameObject == nullptr)
        {
            continue;
        }

        auto cameraComponent = gameObject->GetComponent<CameraComponent>();
        
        if (cameraComponent && cameraComponent->GetTargetTexture())
        {
            rtCameras.push_back( gameObject );
        }
    }
    
    for (auto rtCamera : rtCameras)
    {
        RenderWithCamera( rtCamera );
    }
    
    CameraComponent* camera = mainCamera->GetComponent<CameraComponent>();
    
    if (camera != nullptr)
    {
        RenderWithCamera( mainCamera );
    }
}

void ae3d::Scene::RenderWithCamera( GameObject* cameraGo )
{
    CameraComponent* camera = cameraGo->GetComponent< CameraComponent >();
    GfxDevice::SetRenderTarget( camera->GetTargetTexture() );
    
    const Vec3 color = camera->GetClearColor();
    GfxDevice::SetClearColor( color.x, color.y, color.z );
    GfxDevice::ClearScreen( GfxDevice::ClearFlags::Color | GfxDevice::ClearFlags::Depth );
    GfxDevice::ResetFrameStatistics();

    if (skybox != nullptr)
    {
        Matrix44 view;
        auto cameraTrans = cameraGo->GetComponent< TransformComponent >();
        cameraTrans->GetLocalRotation().GetMatrix( view );
        camera->SetView( view );    
        //Vec3 viewDir = Vec3( view.m[2], view.m[6], view.m[10] ).Normalized();
        //frustum.Update( localPosition, viewDir );
        renderer.RenderSkybox( skybox, *camera );
    }
    
    for (auto gameObject : gameObjects)
    {
        if (gameObject == nullptr)
        {
            continue;
        }
        
        auto transform = gameObject->GetComponent<TransformComponent>();
        
        // TODO: Watch for this duplication of logic.
        
        auto spriteRenderer = gameObject->GetComponent<SpriteRendererComponent>();
        
        if (spriteRenderer)
        {
            Matrix44 projectionModel;
            Matrix44::Multiply( transform ? transform->GetLocalMatrix() : Matrix44::identity, camera->GetProjection(), projectionModel );
            spriteRenderer->Render( projectionModel.m );
        }
        
        auto textRenderer = gameObject->GetComponent<TextRendererComponent>();
        
        if (textRenderer)
        {
            Matrix44 projectionModel;
            Matrix44::Multiply( transform ? transform->GetLocalMatrix() : Matrix44::identity, camera->GetProjection(), projectionModel );
            textRenderer->Render( projectionModel.m );
        }
    }
    
    GfxDevice::ErrorCheck( "Scene render end" );
}

void ae3d::Scene::SetSkybox( const TextureCube* skyTexture )
{
    skybox = skyTexture;

    if (skybox != nullptr && !renderer.IsSkyboxGenerated())
    {
        renderer.GenerateSkybox();
    }
}

std::string ae3d::Scene::GetSerialized() const
{
    std::string outSerialized;

    for (auto gameObject : gameObjects)
    {
        if (gameObject == nullptr)
        {
            continue;
        }

        outSerialized += gameObject->GetSerialized();
        
        // TODO: Try to DRY.
        if (gameObject->GetComponent<TransformComponent>())
        {
            outSerialized += gameObject->GetComponent<TransformComponent>()->GetSerialized();
        }
        if (gameObject->GetComponent<CameraComponent>())
        {
            outSerialized += gameObject->GetComponent<CameraComponent>()->GetSerialized();
        }
        if (gameObject->GetComponent<TextRendererComponent>())
        {
            outSerialized += gameObject->GetComponent<TextRendererComponent>()->GetSerialized();
        }
        if (gameObject->GetComponent<AudioSourceComponent>())
        {
            outSerialized += gameObject->GetComponent<AudioSourceComponent>()->GetSerialized();
        }
    }
    return outSerialized;
}

ae3d::Scene::DeserializeResult ae3d::Scene::Deserialize( const FileSystem::FileContentsData& serialized, std::vector< GameObject >& outGameObjects ) const
{
    outGameObjects.clear();

    std::stringstream stream( std::string( std::begin( serialized.data ), std::end( serialized.data ) ) );
    std::string line;
    
    while (!stream.eof())
    {
        std::getline( stream, line );
        std::stringstream lineStream( line );
        std::string token;
        lineStream >> token;
        
        if (token == "gameobject")
        {
            outGameObjects.push_back( GameObject() );
            std::string name;
            lineStream >> name;
            outGameObjects.back().SetName( name.c_str() );
        }

        if (token == "camera")
        {
            outGameObjects.back().AddComponent< CameraComponent >();
        }

        if (token == "ortho")
        {
            float x, y, width, height, nearp, farp;
            lineStream >> x >> y >> width >> height >> nearp >> farp;
            outGameObjects.back().GetComponent< CameraComponent >()->SetProjection( x, y, width, height, nearp, farp );
        }
        
        if (token == "clearcolor")
        {
            float red, green, blue;
            lineStream >> red >> green >> blue;
            outGameObjects.back().GetComponent< CameraComponent >()->SetClearColor( { red, green, blue } );
        }

        if (token == "transform")
        {
            outGameObjects.back().AddComponent< TransformComponent >();
        }

        if (token == "position")
        {
            float x, y, z;
            lineStream >> x >> y >> z;
            outGameObjects.back().GetComponent< TransformComponent >()->SetLocalPosition( { x, y, z } );
        }

        if (token == "rotation")
        {
            float x, y, z, s;
            lineStream >> x >> y >> z >> s;
            outGameObjects.back().GetComponent< TransformComponent >()->SetLocalRotation( { { x, y, z }, s } );
        }
    }
    
    return DeserializeResult::Success;
}

