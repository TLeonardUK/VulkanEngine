#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform sampler2D gbuffer0;
layout(set = 0, binding = 1) uniform sampler2D gbuffer1;
layout(set = 0, binding = 2) uniform sampler2D gbuffer2;
layout(set = 0, binding = 3) uniform sampler2D shadowMask;
layout(set = 0, binding = 3) uniform sampler2D lightAccumulation;

layout(location = 0) in vec2 inTexCoord1;

layout(location = 0) out vec4 outColor;

void main() 
{
	vec4 diffuse = vec4(texture(gbuffer0, inTexCoord1).rgb, 1.0f);
	float shadowFactor = texture(shadowMask, inTexCoord1).r;
	float lightFactor = texture(lightAccumulation, inTexCoord1).r;
	float brightness = min(1.0f, 0.2f + (lightFactor * shadowFactor)); // add shitty ambient lighting to test with.
	outColor.rgba = diffuse * brightness;
}