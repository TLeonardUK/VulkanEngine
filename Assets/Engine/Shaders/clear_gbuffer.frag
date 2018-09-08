#include "common/constants.h"
#include "common/gbuffer.h"

layout(location = 0) out vec4 gbuffer0;
layout(location = 1) out vec4 gbuffer1;
layout(location = 2) out vec4 gbuffer2;

layout(location = 0) in vec2 inFragPos;

void main() 
{
	GBufferTexel texel;
	texel.albedo = vec3(0.0);
	texel.flags = RenderFlags_None;
	texel.worldNormal = vec3(0.0);
	texel.worldPosition = vec3(0.0);

	writeGBuffer(texel);
} 