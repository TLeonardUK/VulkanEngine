#include "common/constants.h"
#include "common/gbuffer.h"

layout(set = 0, binding = 0) uniform sampler2D gbuffer0;
layout(set = 0, binding = 1) uniform sampler2D gbuffer1;
layout(set = 0, binding = 2) uniform sampler2D gbuffer2;
layout(set = 0, binding = 3) uniform sampler2D shadowMask;
layout(set = 0, binding = 3) uniform sampler2D lightAccumulation;

layout(location = 0) in vec2 inTexCoord1;

layout(location = 0) out vec4 outColor;

void main() 
{
	GBufferTexel texel;
	readGBuffer(texel, inTexCoord1);

	float shadowFactor = texture(shadowMask, inTexCoord1).r;
	float lightFactor = texture(lightAccumulation, inTexCoord1).r;
	
	float brightness = min(1.0f, 0.2f + (lightFactor * shadowFactor)); // add shitty ambient lighting to test with.
	outColor.rgba = vec4(texel.albedo, 1.0f) * brightness;
}