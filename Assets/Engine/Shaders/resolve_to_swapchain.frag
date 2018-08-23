#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform sampler2D gbuffer0;
layout(set = 0, binding = 1) uniform sampler2D gbuffer1;
layout(set = 0, binding = 2) uniform sampler2D gbuffer2;

layout(location = 0) in vec2 inTexCoord1;

layout(location = 0) out vec4 outColor;

void main() 
{
	outColor.rgba = vec4(texture(gbuffer0, inTexCoord1).rgb, 1.0f);
}