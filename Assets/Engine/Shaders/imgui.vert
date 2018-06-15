#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    vec2 scale;
    vec2 translation;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord1;
layout(location = 2) in vec4 inColor;

layout(location = 0) out struct{
    vec4 Color;
    vec2 UV;
} Out;

void main() {
    Out.UV = inTexCoord1;
    Out.Color = inColor;
    gl_Position = vec4(inPosition * ubo.scale + ubo.translation, 0.0, 1.0);
}