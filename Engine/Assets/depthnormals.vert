#version 450 core
    
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;

// Prevents generating code that needs ClipDistance which is not available.
out gl_PerVertex { vec4 gl_Position; };

layout (set = 0, binding = 0) uniform UBO 
{
    uniform mat4 _LocalToClip;
    uniform mat4 _LocalToView;
} ubo;

layout (location = 0) out vec3 vPositionInView;
layout (location = 1) out vec3 vNormalInView;
    
void main()
{
    gl_Position = ubo._LocalToClip * vec4( aPosition, 1.0 );
    vPositionInView = (ubo._LocalToView * vec4( aPosition, 1.0 )).xyz;
    vNormalInView = (ubo._LocalToView * vec4( aNormal, 0.0 )).xyz;
}
    