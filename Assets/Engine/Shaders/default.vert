#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} properties;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord1;

layout(location = 0) out vec3 outWorldPosition;
layout(location = 1) out vec3 outWorldNormal;
layout(location = 2) out vec2 outTexCoord0;

void main() 
{
    outTexCoord0 = inTexCoord1;
    outWorldNormal = normalize(properties.model * vec4(inNormal, 1.0)).xyz;
    outWorldPosition = (properties.model * vec4(inPosition, 1.0)).xyz;

    gl_Position = properties.proj * properties.view * properties.model * vec4(inPosition, 1.0);
}