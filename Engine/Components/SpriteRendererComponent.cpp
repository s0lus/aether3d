#include "SpriteRendererComponent.hpp"
#include <vector>
#include <algorithm>
//#include <cassert>
#include "Renderer.hpp"
#include "Texture2D.hpp"
#include "Vec3.hpp"
#include "VertexBuffer.hpp"
#include "System.hpp"

extern ae3d::Renderer renderer;

ae3d::SpriteRendererComponent spriteRendererComponents[100];
int nextFreeSpriteComponent = 0;

struct Drawable
{
    ae3d::Texture2D* texture = nullptr;
    int bufferStart = 0;
    int bufferEnd = 0;
};

struct Sprite
{
    ae3d::Texture2D* texture = nullptr;
    ae3d::Vec3 position;
    ae3d::Vec3 dimension;
    ae3d::Vec4 tint;
};

struct ae3d::SpriteRendererComponent::Impl
{
    void BuildRenderQueue();
    void CreateVertexBuffer();
    void CreateDrawables();
    void Render();

    bool renderQueueIsDirty = true;
    std::vector< Sprite > sprites;
    std::vector< Drawable > drawables;
    VertexBuffer vertexBuffer;
};

void ae3d::SpriteRendererComponent::Impl::CreateVertexBuffer()
{
    std::vector< VertexBuffer::VertexPTC > vertices(sprites.size() * 4);
    std::vector< VertexBuffer::Face > faces(sprites.size() * 2);

    // TODO: evaluate perf and consider other containers.
    std::sort( sprites.begin(), sprites.end(), [](const Sprite& a, const Sprite& b) { return a.texture->GetID() <= b.texture->GetID(); } );

    for (unsigned short i = 0; i < static_cast<unsigned short>(sprites.size()); ++i)
    {
        vertices[ i * 4 + 0 ].position = sprites[i].position;
        vertices[ i * 4 + 1 ].position = sprites[i].position + Vec3( sprites[i].dimension.x, 0, 0 );
        vertices[ i * 4 + 2 ].position = sprites[i].position + Vec3( sprites[i].dimension.x, sprites[i].dimension.y, 0 );
        vertices[ i * 4 + 3 ].position = sprites[i].position + Vec3( 0, sprites[i].dimension.y, 0 );

        for (int v = 0; v < 4; ++v)
        {
            vertices[ i * 4 + v ].color = sprites[i].tint;
        }

        vertices[ i * 4 + 0 ].u = 0;
        vertices[ i * 4 + 0 ].v = 0;

        vertices[ i * 4 + 1 ].u = 1;
        vertices[ i * 4 + 1 ].v = 0;

        vertices[ i * 4 + 2 ].u = 1;
        vertices[ i * 4 + 2 ].v = 1;

        vertices[ i * 4 + 3 ].u = 0;
        vertices[ i * 4 + 3 ].v = 1;

        auto& tri1 = faces[ i * 2 + 0 ];
        tri1.a = i * 4 + 0;
        tri1.b = i * 4 + 1;
        tri1.c = i * 4 + 2;

        auto& tri2 = faces[ i * 2 + 1 ];
        tri2.a = i * 4 + 2;
        tri2.b = i * 4 + 3;
        tri2.c = i * 4 + 0;
    }

    vertexBuffer.Generate(faces.data(), static_cast<int>(faces.size()), vertices.data(), static_cast<int>(vertices.size()));
}

void ae3d::SpriteRendererComponent::Impl::CreateDrawables()
{
    drawables.clear();

    if (sprites.empty())
    {
        return;
    }

    drawables.push_back(Drawable());
    auto& back = drawables.back();
    back.texture = sprites[0].texture;
    back.bufferStart = 0;
    back.bufferEnd = 2;

    for (std::size_t s = 1; s < sprites.size(); ++s)
    {
        if (sprites[s].texture == sprites[s - 1].texture)
        {
            drawables.back().bufferEnd += 2;
        }
        else
        {
            int oldEnd = drawables.back().bufferEnd;

            drawables.push_back(Drawable());
            auto& back2 = drawables.back();
            back2.texture = sprites[s].texture;
            back2.bufferStart = oldEnd;
            back2.bufferEnd = oldEnd + 2;
        }
    }
}

void ae3d::SpriteRendererComponent::Impl::Render()
{
    if (renderQueueIsDirty)
    {
        BuildRenderQueue();
    }

    vertexBuffer.Bind();

    for (auto& drawable : drawables)
    {
        renderer.builtinShaders.spriteRendererShader.SetTexture("textureMap", drawable.texture, 0);
        vertexBuffer.DrawRange(drawable.bufferStart, drawable.bufferEnd);
    }
}

int ae3d::SpriteRendererComponent::New()
{
    return nextFreeSpriteComponent++;
}

ae3d::SpriteRendererComponent* ae3d::SpriteRendererComponent::Get(int index)
{
    return &spriteRendererComponents[index];
}

ae3d::SpriteRendererComponent::SpriteRendererComponent()
{
    new(&_storage)Impl();
    //static_assert(sizeof(Impl) <= StorageSize && StorageSize <= sizeof(Impl) * 1.1,
    //    "SpriteRendererComponent::StorageSize need be changed");
    //static_assert(StorageAlign == alignof(Impl),
    //    "SpriteRendererComponent::StorageAlign need be changed")
}

void ae3d::SpriteRendererComponent::Clear()
{
    m().sprites.clear();
    m().renderQueueIsDirty = true;
}

void ae3d::SpriteRendererComponent::SetTexture( Texture2D* aTexture, const Vec3& position, const Vec3& dimensionPixels, const Vec4& tintColor )
{
    Sprite sprite;
    sprite.texture = aTexture;
    sprite.position = position;
    sprite.dimension = dimensionPixels;
    sprite.tint = tintColor;
    m().sprites.emplace_back( sprite );
    m().renderQueueIsDirty = true;
}

void ae3d::SpriteRendererComponent::Impl::BuildRenderQueue()
{
    CreateVertexBuffer();
    CreateDrawables();
    renderQueueIsDirty = false;
}

void ae3d::SpriteRendererComponent::Render( const float* projectionMatrix )
{    
    renderer.builtinShaders.spriteRendererShader.Use();
    renderer.builtinShaders.spriteRendererShader.SetMatrix( "_ProjectionMatrix", projectionMatrix );
 
    m().Render();
}
