#include "Renderer.hpp"
#include "FileSystem.hpp"

ae3d::Renderer renderer;

void ae3d::BuiltinShaders::Load()
{
    spriteRendererShader.LoadSPIRV( FileSystem::FileContents( "sprite_vert.spv" ), FileSystem::FileContents( "sprite_frag.spv" ) );
    sdfShader.LoadSPIRV( FileSystem::FileContents( "sprite_vert.spv" ), FileSystem::FileContents( "sprite_frag.spv" ) );
    skyboxShader.LoadSPIRV( FileSystem::FileContents( "skybox_vert.spv" ), FileSystem::FileContents( "skybox_frag.spv" ) );
    momentsShader.LoadSPIRV( FileSystem::FileContents( "moments_vert.spv" ), FileSystem::FileContents( "moments_frag.spv" ) );
    momentsSkinShader.LoadSPIRV( FileSystem::FileContents( "moments_skin_vert.spv" ), FileSystem::FileContents( "moments_frag.spv" ) );
    depthNormalsShader.LoadSPIRV( FileSystem::FileContents( "depthnormals_vert.spv" ), FileSystem::FileContents( "depthnormals_frag.spv" ) );
    uiShader.LoadSPIRV( FileSystem::FileContents( "sprite_vert.spv" ), FileSystem::FileContents( "sprite_frag.spv" ) );
}
