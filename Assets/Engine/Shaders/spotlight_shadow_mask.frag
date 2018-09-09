#include "common/random.h"
#include "common/constants.h"
#include "common/gbuffer.h"

#define FILTER_PCF 1

layout(set = 0, binding = 0) uniform sampler2D gbuffer0;
layout(set = 0, binding = 1) uniform sampler2D gbuffer1;
layout(set = 0, binding = 2) uniform sampler2D gbuffer2;
layout(set = 1, binding = 0) uniform sampler2D lightShadowMap;
layout(set = 2, binding = 0) uniform LightPropertiesBlock {
	vec4 lightWorldDirection;
	float lightShadowMapSize;
    mat4 lightShadowViewProj;
} lightProperties;

layout(location = 0) in FragInputData {
    vec2 texCoord;
} inFragData; 
    
layout(location = 0) out vec4 outColor;

float poissonSample(vec2 uv, float clipSpaceZ, vec4 worldPosition)
{
	float result = 0.0;
		
#if FILTER_PCF

	const int KernelSize = 2;

	vec2 texelSize = (vec2(1.0) / lightProperties.lightShadowMapSize);
	
	int i = 0;
	for (int y = -KernelSize; y <= KernelSize; y++)
	{
		for (int x = -KernelSize; x <= KernelSize; x++)
		{	
			vec2 rotation = vec2(
				cos(random(vec4(worldPosition.xyz, ++i))),
				sin(random(vec4(worldPosition.xyz, ++i)))
			);
			
			vec2 poissonOffset = vec2(
				rotation.x * x - rotation.y * y,
				rotation.y * x + rotation.x * y
			);
			
			vec2 sampleUV = uv + (poissonOffset * texelSize);
						
				
			float shadowMapDepth = textureLod(lightShadowMap, sampleUV, 0.0).r;
			result += (shadowMapDepth < clipSpaceZ) ? 0.0 : 1.0;
		}
	}
	
	result /= ((KernelSize + 1) * (KernelSize + 1));

#else

	float shadowMapDepth = textureLod(lightShadowMap, uv, 0.0).r;
	result += (shadowMapDepth < clipSpaceZ) ? 0.0 : 1.0;

#endif
		
	return result;
}

float sampleShadowMap(vec4 worldPosition)
{		
	vec4 worldPositionLightSpace = lightProperties.lightShadowViewProj * worldPosition;
	vec4 worldPositionLightClipSpace = worldPositionLightSpace / worldPositionLightSpace.w;
		
	if (worldPositionLightClipSpace.z > -1.0 && worldPositionLightClipSpace.z < 1.0)
	{
		vec2 shadowMapCoord = worldPositionLightClipSpace.xy * 0.5 + 0.5;			
		if (shadowMapCoord.x >= 0.0 && shadowMapCoord.y >= 0.0 && shadowMapCoord.x <= 1.0 && shadowMapCoord.y <= 1.0)
		{			
			return poissonSample(shadowMapCoord, worldPositionLightClipSpace.z, worldPosition);
		}
	}
	
	return 1.0;
}

void main() 
{
	GBufferTexel texel;
	readGBuffer(texel, inFragData.texCoord);
	
	float unshadowedFraction = 1.0;
	
	// Check this pixel is a shadow reciever.
	if ((texel.flags & RenderFlags_ShadowReciever) != 0)
	{	
		unshadowedFraction = sampleShadowMap(vec4(texel.worldPosition, 1.0));	
	}

	outColor.rgba = vec4(unshadowedFraction, 0.0, 0.0, 1.0);	
}




