#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform GlobalPropertiesBlock{
    mat4 view;
    mat4 proj;
} globalProperties;

layout(set = 0, binding = 0) uniform MeshPropertiesBlock {
    mat4 model;
} meshProperties;

layout(location = 0) in vec3 inPosition;

void main() 
{
    gl_Position = globalProperties.proj * globalProperties.view * meshProperties.model * vec4(inPosition, 1.0);
}