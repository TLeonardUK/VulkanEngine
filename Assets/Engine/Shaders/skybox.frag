#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 camPosition;
} ubo;

layout(binding = 1) uniform samplerCube albedoTextureSampler;

layout(location = 0) in vec3 inWorldPosition;
layout(location = 1) in vec3 inWorldNormal;
layout(location = 2) in vec2 inTexCoord1;

layout(location = 0) out vec4 gbuffer0;
layout(location = 1) out vec4 gbuffer1;
layout(location = 2) out vec4 gbuffer2;

void main() 
{
    vec3 I = normalize(inWorldPosition - ubo.camPosition);
    vec3 R = reflect(I, normalize(inWorldNormal));
    
    gbuffer0.rgba = vec4(texture(albedoTextureSampler, I).rgb, 1.0);
    gbuffer1.rgba = vec4(inWorldNormal.rgb, 1.0);
    gbuffer2.rgba = vec4(inWorldPosition.rgb, 1.0);
}