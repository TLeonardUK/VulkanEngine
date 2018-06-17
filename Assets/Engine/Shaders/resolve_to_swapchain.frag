#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D gbuffer0;
layout(binding = 2) uniform sampler2D gbuffer1;
layout(binding = 3) uniform sampler2D gbuffer2;

layout(location = 0) in vec2 inTexCoord1;

layout(location = 0) out vec4 outColor;

void main() 
{
	outColor.rgba = texture(gbuffer0, inTexCoord1);
}