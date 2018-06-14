#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 camPosition;
} ubo;

layout(binding = 1) uniform samplerCube skyboxSampler;

layout(location = 0) in vec3 fragPosition;
layout(location = 3) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

void main() 
{
    vec3 I = normalize(fragPosition - ubo.camPosition);
    vec3 R = reflect(I, normalize(fragNormal));
    outColor = vec4(texture(skyboxSampler, I).rgb, 1.0);
}