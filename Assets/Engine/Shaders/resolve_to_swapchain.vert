#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord1;

layout(location = 0) out vec2 outTexCoord1;

void main() 
{
    outTexCoord1 = inTexCoord1;
    gl_Position = vec4(inPosition, 0.0, 1.0);
}