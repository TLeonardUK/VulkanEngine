#version 450
#extension GL_ARB_separate_shader_objects : enable

const int MaxCascades = 8;

layout(set = 0, binding = 0) uniform sampler2D gbuffer2;
layout(set = 1, binding = 0) uniform sampler2D cascadeShadowMaps[MaxCascades];
layout(set = 2, binding = 0) uniform LightPropertiesBlock {
	int cascadeCount;
	vec4 viewPosition;
    float cascadeSplitDistance[MaxCascades];
    mat4 cascadeViewProj[MaxCascades];
} lightProperties;

layout(location = 0) in FragInputData {
    vec2 texCoord;
} inFragData;

layout(location = 0) out vec4 outColor;

vec2 poissonDisk[16] = vec2[](
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

float random(vec4 seed)
{   
	float dot_product = dot(seed, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}

float poissonSample(int cascadeIndex, vec2 uv, float clipSpaceZ, vec4 worldPosition)
{
	float visibility = 1.0f;

	float shadowMapDepth = texture(cascadeShadowMaps[cascadeIndex], uv).r;
	if (shadowMapDepth < clipSpaceZ)
	{
		visibility = 0.0f;
	}
	
	/*
	for (int i = 0; i < 4; i++)
	{		
		int index = int(16.0 * random(vec4(worldPosition.xyz, i))) % 16;
		float shadowMapDepth = texture(cascadeShadowMaps[cascadeIndex], uv + poissonDisk[index] / 1500.0f).r;
		if (shadowMapDepth < clipSpaceZ)
		{
			visibility -= 0.2f;
		}
	}*/
	
	return visibility;
}

int getCascadeIndex(vec4 worldPosition)
{
	/*float distance = length(worldPosition - lightProperties.viewPosition);
	for (int i = 0; i < lightProperties.cascadeCount; i++)
	{
		if (distance < lightProperties.cascadeSplitDistance[i])
		{
			return i;
		}
	}*/
	
	
	for (int i = 0; i < lightProperties.cascadeCount; i++)
	{
		vec4 worldPositionLightSpace = lightProperties.cascadeViewProj[i] * worldPosition;
		vec4 worldPositionLightClipSpace = worldPositionLightSpace / worldPositionLightSpace.w;
		
		if (worldPositionLightClipSpace.z > -1.0f && worldPositionLightClipSpace.z < 1.0f)
		{
			vec2 shadowMapCoord = worldPositionLightClipSpace.xy * 0.5 + 0.5;	
			if (shadowMapCoord.x >= 0.0f && shadowMapCoord.y >= 0.0f && shadowMapCoord.x < 1.0f && shadowMapCoord.y < 1.0f)
			{
				return i;
			}
		}
	}	
	
	return lightProperties.cascadeCount - 1;
}

void main() 
{
	vec4 worldPosition = vec4(texture(gbuffer2, inFragData.texCoord).xyz, 1.0f);

	float result = 1.0f;
	
	int cascadeIndex = getCascadeIndex(worldPosition);	

	/*
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
	*/
	
	vec4 worldPositionLightSpace = lightProperties.cascadeViewProj[cascadeIndex] * worldPosition;
	vec4 worldPositionLightClipSpace = worldPositionLightSpace / worldPositionLightSpace.w;
	
	if (worldPositionLightClipSpace.z > -1.0f && worldPositionLightClipSpace.z < 1.0f)
	{
		vec2 shadowMapCoord = worldPositionLightClipSpace.xy * 0.5 + 0.5;	
		
		result = poissonSample(cascadeIndex, shadowMapCoord, worldPositionLightClipSpace.z, worldPosition);
	}
	
	outColor.rgba = vec4(result, 0.0f, 0.0f, 1.0f);	
}




