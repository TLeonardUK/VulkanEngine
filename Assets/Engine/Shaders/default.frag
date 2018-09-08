#include "common/constants.h"
#include "common/gbuffer.h"

layout(set = 0, binding = 0) uniform MeshPropertiesBlock {
    mat4 model;
	int flags;
} meshProperties;

layout(set = 2, binding = 0) uniform sampler2D albedoTextureSampler;

layout(location = 0) in vec4 inWorldPosition;
layout(location = 1) in vec4 inWorldNormal;
layout(location = 2) in vec2 inTexCoord1;

layout(location = 0) out vec4 gbuffer0;
layout(location = 1) out vec4 gbuffer1;
layout(location = 2) out vec4 gbuffer2;

void main() 
{
	GBufferTexel texel;
	texel.albedo = texture(albedoTextureSampler, inTexCoord1).rgb;
	texel.flags = meshProperties.flags;
	texel.worldNormal = normalize(inWorldNormal).xyz;
	texel.worldPosition = inWorldPosition.xyz;

	writeGBuffer(texel);
}