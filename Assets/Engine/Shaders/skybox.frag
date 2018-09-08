#include "common/constants.h"
#include "common/gbuffer.h"

layout(set = 0, binding = 0) uniform MeshPropertiesBlock {
    mat4 model;
	int flags;
} meshProperties;

layout(set = 1, binding = 0) uniform GlobalPropertiesBlock {
    mat4 view;
    mat4 proj;
    vec3 camPosition;
} globalProperties;

layout(set = 2, binding = 0) uniform samplerCube albedoTextureSampler;

layout(location = 0) in vec4 inWorldPosition;
layout(location = 1) in vec4 inWorldNormal;
layout(location = 2) in vec2 inTexCoord1;

layout(location = 0) out vec4 gbuffer0;
layout(location = 1) out vec4 gbuffer1;
layout(location = 2) out vec4 gbuffer2;

void main() 
{
    vec3 I = normalize(inWorldPosition.xyz - globalProperties.camPosition);
    vec3 R = reflect(I, normalize(inWorldNormal).xyz);
    
	GBufferTexel texel;
	texel.albedo = texture(albedoTextureSampler, I).rgb;
	texel.flags = meshProperties.flags;
	texel.worldNormal = normalize(inWorldNormal).xyz;
	texel.worldPosition = inWorldPosition.xyz;

	writeGBuffer(texel);
}