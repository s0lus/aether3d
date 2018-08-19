#include "Inspector.hpp"
#include "AudioSourceComponent.hpp"
#include "CameraComponent.hpp"
#include "GameObject.hpp"
#include "MeshRendererComponent.hpp"
#include "System.hpp"
#include "Texture2D.hpp"
#include "TransformComponent.hpp"
#include <cstring>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#if RENDERER_METAL
#include "nuklear.h"
#else
#include "../../Samples/NuklearTest/nuklear.h"
#endif

using namespace ae3d;

struct VertexPTC
{
    float position[ 3 ];
    float uv[ 2 ];
    float col[ 4 ];
};

nk_draw_null_texture nullTexture;
Texture2D* uiTextures[ 1 ];
nk_context ctx;
nk_font_atlas atlas;
int atlasWidth = 0;
int atlasHeight = 0;
Texture2D nkFontTexture;

void DrawNuklear( int width, int height )
{
    nk_convert_config config = {};
    static const nk_draw_vertex_layout_element vertex_layout[] = {
        {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF( VertexPTC, position )},
        {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF( VertexPTC, uv )},
        {NK_VERTEX_COLOR, NK_FORMAT_R32G32B32A32_FLOAT, NK_OFFSETOF( VertexPTC, col )},
        {NK_VERTEX_LAYOUT_END}
    };

    config.vertex_layout = vertex_layout;
    config.vertex_size = sizeof( VertexPTC );
    config.vertex_alignment = NK_ALIGNOF( VertexPTC );
    config.null = nullTexture;
    config.circle_segment_count = 22;
    config.curve_segment_count = 22;
    config.arc_segment_count = 22;
    config.global_alpha = 1.0f;
    config.shape_AA = NK_ANTI_ALIASING_OFF;
    config.line_AA = NK_ANTI_ALIASING_OFF;

    const int MAX_VERTEX_MEMORY = 512 * 1024;
    const int MAX_ELEMENT_MEMORY = 128 * 1024;

    void* vertices;
    void* elements;
    System::MapUIVertexBuffer( MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY, &vertices, &elements );
    std::memset( vertices, 0, MAX_VERTEX_MEMORY * sizeof( VertexPTC ) );
    std::memset( elements, 0, MAX_ELEMENT_MEMORY * 3 * 2 );
    
    nk_buffer vbuf, ebuf, uiCommands;
    nk_buffer_init_default( &uiCommands );
    nk_buffer_init_fixed( &vbuf, vertices, MAX_VERTEX_MEMORY * sizeof( VertexPTC ) );
    nk_buffer_init_fixed( &ebuf, elements, MAX_ELEMENT_MEMORY * 3 * 2 );
    const auto res = nk_convert( &ctx, &uiCommands, &vbuf, &ebuf, &config );
    System::Assert( res == NK_CONVERT_SUCCESS, "buffer conversion failed!" );
    
    System::UnmapUIVertexBuffer();
    
    const struct nk_draw_command* cmd = nullptr;
    nk_draw_index* offset = nullptr;

    nk_draw_foreach( cmd, &ctx, &uiCommands )
    {
        if (cmd->elem_count == 0)
        {
            continue;
        }

        System::DrawUI( (int)(cmd->clip_rect.x),
                        (int)((height - (int)(cmd->clip_rect.y + cmd->clip_rect.h))),
                        (int)(cmd->clip_rect.w),
                        (int)(cmd->clip_rect.h),
                        cmd->elem_count, uiTextures[ 0/*cmd->texture.id*/ ], offset, width, height );
        offset += cmd->elem_count;
    }

    nk_clear( &ctx );
}

void Inspector::Init()
{    
    nk_font_atlas_init_default( &atlas );
    nk_font_atlas_begin( &atlas );

    nk_font* nkFont = nk_font_atlas_add_default( &atlas, 13.0f, nullptr );
    const void* image = nk_font_atlas_bake( &atlas, &atlasWidth, &atlasHeight, NK_FONT_ATLAS_RGBA32 );

    nkFontTexture.LoadFromData( image, atlasWidth, atlasHeight, 4, "Nuklear font" );
    nk_font_atlas_end( &atlas, nk_handle_id( nkFontTexture.GetID() ), &nullTexture );
    
    uiTextures[ 0 ] = &nkFontTexture;
    
    nk_init_default( &ctx, &nkFont->handle );
}

void Inspector::Render( int width, int height )
{
    if (nk_begin( &ctx, "Inspector", nk_rect( 0, 50, 300, 400 ), NK_WINDOW_BORDER | NK_WINDOW_TITLE ))
    {
        nk_layout_row_static( &ctx, 30, 150, 1 );

        const char* str = gameObject ? gameObject->GetName().c_str() : "object name";

        nk_label( &ctx, str, NK_TEXT_LEFT );

        TransformComponent* transform = gameObject ? gameObject->GetComponent< TransformComponent >() : nullptr;
        MeshRendererComponent* meshRenderer = gameObject ? gameObject->GetComponent< MeshRendererComponent >() : nullptr;
        AudioSourceComponent* audioSource = gameObject ? gameObject->GetComponent< AudioSourceComponent >() : nullptr;
        CameraComponent* camera = gameObject ? gameObject->GetComponent< CameraComponent >() : nullptr;
        
        if (transform != nullptr)
        {
            Vec3 pos = transform->GetLocalPosition();
            
            nk_property_float( &ctx, "#X:", -1024.0f, &pos.x, 1024.0f, 1, 1 );
            nk_property_float( &ctx, "#Y:", -1024.0f, &pos.y, 1024.0f, 1, 1 );
            nk_property_float( &ctx, "#Z:", -1024.0f, &pos.z, 1024.0f, 1, 1 );
        }
        
        if (meshRenderer == nullptr && nk_button_label( &ctx, "add mesh renderer component" ))
        {
            gameObject->AddComponent< MeshRendererComponent >();
            System::Print("Added a mesh renderer\n");
        }

        if (audioSource == nullptr && nk_button_label( &ctx, "add audio source component" ))
        {
            gameObject->AddComponent< AudioSourceComponent >();
            System::Print("Added audiosource\n");
        }

        if (camera == nullptr && nk_button_label( &ctx, "add camera component" ))
        {
            gameObject->AddComponent< CameraComponent >();
            System::Print("Added camera\n");
        }

        DrawNuklear( width, height );
        nk_end( &ctx );
    }
}

void Inspector::Deinit()
{
    nk_font_atlas_clear( &atlas );
    nk_free( &ctx );
}
