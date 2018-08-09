#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 1) uniform sampler2D albedoTextureSampler;

layout(location = 0) in vec3 inWorldPosition;
layout(location = 1) in vec3 inWorldNormal;
layout(location = 2) in vec2 inTexCoord1;

layout(location = 0) out vec4 gbuffer0;
layout(location = 1) out vec4 gbuffer1;
layout(location = 2) out vec4 gbuffer2;

void main() 
{
    gbuffer0.rgba = texture(albedoTextureSampler, inTexCoord1);
    gbuffer1.rgba = vec4(inWorldNormal.rgb, 1.0);
    gbuffer2.rgba = vec4(inWorldPosition.rgb, 1.0);

}