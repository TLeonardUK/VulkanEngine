#include "common/random.h"
#include "common/constants.h"
#include "common/gbuffer.h"

#define VISUALIZE_CASCADES 0
#define FILTER_PCF 1
#define FILTER_POISSON_DISK 0
#define DO_CASCADE_BLENDING 1

layout(set = 0, binding = 0) uniform sampler2D gbuffer0;
layout(set = 0, binding = 1) uniform sampler2D gbuffer1;
layout(set = 0, binding = 2) uniform sampler2D gbuffer2;
layout(set = 1, binding = 0) uniform sampler2D cascadeShadowMaps[MaxShadowCascades];
layout(set = 2, binding = 0) uniform LightPropertiesBlock {
	int cascadeCount;
	float cascadeBlendFactor;
	vec4 viewPosition;
	vec4 lightWorldDirection;
	float cascadeMapSize[MaxShadowCascades];
    float cascadeSplitDistance[MaxShadowCascades];
    float cascadeRadius[MaxShadowCascades];
    mat4 cascadeViewProj[MaxShadowCascades];
} lightProperties;

layout(location = 0) in FragInputData {
    vec2 texCoord;
} inFragData; 
    
layout(location = 0) out vec4 outColor;

 
float depthBias[8] = float[]
(
	0.0010f,
	0.0015f,
	0.0015f,
	0.0015f,
	0.0015f,
	0.0015f,
	0.0015f,
	0.0015f
);

float worldRotationScale[8] = float[]
(
	1.0f / 1.0f,
	1.0f / 2.0f,
	1.0f / 3.0f,
	1.0f / 4.0f,
	1.0f / 5.0f,
	1.0f / 6.0f,
	1.0f / 7.0f,
	1.0f / 8.0f
);


float poissonSample(int cascadeIndex, vec2 uv, float clipSpaceZ, vec4 worldPosition)
{
	float result = 0.0;
		
#if FILTER_POISSON_DISK

	const int numSamples = 16;
	
	float uvMultiplier = 3.0f;
	vec2 texelSize = (vec2(1.0) / lightProperties.cascadeMapSize[cascadeIndex]) * uvMultiplier;
		
	vec4 roundedWorldPosition =  worldPosition;//floor(worldPosition / scale);
	
	for (int s = 0; s < numSamples; ++s)
	{
		int index = int(16.0 * random(vec4(roundedWorldPosition.xyz, s))) % 16;
		
		vec2 rotation = vec2(
			cos(random(vec4(roundedWorldPosition.xyz, s + 10))),
			sin(random(vec4(roundedWorldPosition.xyz, s + 20)))
		);
		
		vec2 poissonOffset = vec2(
			rotation.x * PoissonDisk[index].x - rotation.y * PoissonDisk[index].y,
			rotation.y * PoissonDisk[index].x + rotation.x * PoissonDisk[index].y
		);
		
		vec2 sampleUV = uv - (texelSize*0.5) + (poissonOffset * texelSize * worldRotationScale[cascadeIndex]);
		
		//float bias = 0.005 * tan(acos(NoL));
		
		float shadowMapDepth = textureLod(cascadeShadowMaps[cascadeIndex], sampleUV, 0.0).r;
		result += (shadowMapDepth < clipSpaceZ/* - depthBias[cascadeIndex]*/) ? 0.0 : 1.0;
	}
	
	/*for (int s = 0; s < numSamples; ++s)
	{
		int index = s % 16;
				
		vec2 poissonOffset = PoissonDisk[index];
		
		vec2 sampleUV = uv - (texelSize*0.5) + (poissonOffset * texelSize);
		
		float shadowMapDepth = textureLod(cascadeShadowMaps[cascadeIndex], sampleUV, 0.0).r;
		result += (shadowMapDepth < clipSpaceZ) ? 0.0 : 1.0;
	}*/
	
	result = result / numSamples;
	
	/*if (result > 0.8f)
	{
		result = 0.8f;
	}*/
	
#elif FILTER_PCF

	const int KernelSize = 2;

	//float uvMultiplier = (lightProperties.cascadeRadius[0] / lightProperties.cascadeRadius[cascadeIndex]) * 100f;
	vec2 texelSize = (vec2(1.0) / lightProperties.cascadeMapSize[cascadeIndex]);
	
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
						
				
			float shadowMapDepth = textureLod(cascadeShadowMaps[cascadeIndex], sampleUV, 0.0).r;
			result += (shadowMapDepth < clipSpaceZ) ? 0.0 : 1.0;
		}
	}
	
	result /= ((KernelSize + 1) * (KernelSize + 1));

#else

	float shadowMapDepth = textureLod(cascadeShadowMaps[cascadeIndex], uv, 0.0).r;
	result += (shadowMapDepth < clipSpaceZ) ? 0.0 : 1.0;

#endif
		
	return result;
}

struct CascadeInfo
{
	int primaryIndex;	
	int secondaryIndex;	
	float blendFactor;
};

CascadeInfo getCascadeInfo(vec4 worldPosition)
{
	CascadeInfo info;
	info.blendFactor = 1.0;	
	info.primaryIndex = lightProperties.cascadeCount - 1;
	info.secondaryIndex = info.primaryIndex + 1;
	
	for (int i = 0; i < lightProperties.cascadeCount; i++)
	{
		vec4 worldPositionLightSpace = lightProperties.cascadeViewProj[i] * worldPosition;
		vec4 worldPositionLightClipSpace = worldPositionLightSpace / worldPositionLightSpace.w;
		
		if (worldPositionLightClipSpace.z > -1.0 && worldPositionLightClipSpace.z < 1.0)
		{
			vec2 shadowMapCoord = worldPositionLightClipSpace.xy * 0.5 + 0.5;	
			if (shadowMapCoord.x >= 0.0 && shadowMapCoord.y >= 0.0 && shadowMapCoord.x <= 1.0 && shadowMapCoord.y <= 1.0)
			{			
				float minDistance = min(min(min(
					 shadowMapCoord.x,
					 1.0 - shadowMapCoord.x),
					 shadowMapCoord.y),
					 1.0 - shadowMapCoord.y);
				
				info.blendFactor = 1.0f - clamp(minDistance / lightProperties.cascadeBlendFactor, 0.0, 1.0);
				
				info.primaryIndex = i;
				info.secondaryIndex = i + 1;
				
				break;
			}
		}
	}	
	
	return info;	
}

float sampleShadowMap(int cascadeIndex, vec4 worldPosition)
{		
	vec4 worldPositionLightSpace = lightProperties.cascadeViewProj[cascadeIndex] * worldPosition;
	vec4 worldPositionLightClipSpace = worldPositionLightSpace / worldPositionLightSpace.w;
		
	if (worldPositionLightClipSpace.z > -1.0 && worldPositionLightClipSpace.z < 1.0)
	{
		vec2 shadowMapCoord = worldPositionLightClipSpace.xy * 0.5 + 0.5;			
		return poissonSample(cascadeIndex, shadowMapCoord, worldPositionLightClipSpace.z, worldPosition);
	}
	
	return 1.0;
}

void main() 
{
	GBufferTexel texel;
	readGBuffer(texel, inFragData.texCoord);
	
	float unshadowedFraction = 1.0;
	
	// Get the cascade we should sample from.
	CascadeInfo cascadeInfo = getCascadeInfo(vec4(texel.worldPosition, 1.0f));
	
#if VISUALIZE_CASCADES

	if (cascadeIndex == 0)
	{
		outColor.rgba = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (cascadeIndex == 1)
	{
		outColor.rgba = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	}
	else if (cascadeIndex == 2)
	{
		outColor.rgba = vec4(0.0f, 0.0f, 1.0f, 1.0f);
	}
	else 
	{
		outColor.rgba = vec4(1.0f, 0.0f, 1.0f, 1.0f);
	}

#else
	
	// Check this pixel is a shadow reciever.
	if ((texel.flags & RenderFlags_ShadowReciever) != 0)
	{	
#if DO_CASCADE_BLENDING
		float primaryResult = 0.0;
		float secondaryResult = 0.0;
		
		// If normal is in opposite direction from light, we know already that we are in shadow.
		if (dot(vec4(texel.worldNormal, 1.0f), lightProperties.lightWorldDirection) <= 0.0)
		{
			unshadowedFraction = 0.0f;
		}
		else
		{
			// Sample from primary and secondary cascade if we are on a blending band.
			if (cascadeInfo.blendFactor >= 0.0 && cascadeInfo.blendFactor < 1.0)
			{
				primaryResult = sampleShadowMap(cascadeInfo.primaryIndex, vec4(texel.worldPosition, 1.0));
			}	

			if (cascadeInfo.secondaryIndex < lightProperties.cascadeCount)
			{
				if (cascadeInfo.blendFactor > 0.0 && cascadeInfo.blendFactor <= 1.0)
				{
					secondaryResult = sampleShadowMap(cascadeInfo.secondaryIndex, vec4(texel.worldPosition, 1.0));	
				}
			}
			else
			{
				secondaryResult = 1.0;
			}
			
			// Mix each cascade depending on how far into the blending band we are.
			unshadowedFraction = mix(primaryResult, secondaryResult, cascadeInfo.blendFactor);
		}		
#else
		unshadowedFraction = sampleShadowMap(cascadeInfo.primaryIndex, vec4(texel.worldPosition, 1.0));	
#endif
	}

	outColor.rgba = vec4(unshadowedFraction, 0.0, 0.0, 1.0);	

#endif
}




