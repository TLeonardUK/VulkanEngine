#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 gbuffer0;
layout(location = 1) out vec4 gbuffer1;
layout(location = 2) out vec4 gbuffer2;

layout(location = 0) in vec2 inFragPos;

void main() 
{
    gbuffer0.rgba = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    gbuffer1.rgba = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    gbuffer2.rgba = vec4(0.0f, 0.0f, 0.0f, 0.0f);

}