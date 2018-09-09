#include "common/random.h"
#include "common/constants.h"
#include "common/gbuffer.h"

#define FILTER_PCF 0

layout(set = 0, binding = 0) uniform sampler2D gbuffer0;
layout(set = 0, binding = 1) uniform sampler2D gbuffer1;
layout(set = 0, binding = 2) uniform sampler2D gbuffer2;
layout(set = 1, binding = 0) uniform samplerCube lightShadowMap;
layout(set = 2, binding = 0) uniform LightPropertiesBlock {
	float lightShadowMapSize;
	float lightMaxDistance;
	vec4 lightWorldLocation;
} lightProperties;

layout(location = 0) in FragInputData {
    vec2 texCoord;
} inFragData; 
    
layout(location = 0) out vec4 outColor;

float sampleShadowMap(vec4 worldPosition)
{		
    vec3 fragToLight = worldPosition.xyz - lightProperties.lightWorldLocation.xyz;	
    float closestDepth = texture(lightShadowMap, fragToLight).r * lightProperties.lightMaxDistance;    
	float currentDepth = length(fragToLight);
	
    float bias = 0.05; 
    float shadow = closestDepth < currentDepth - bias ? 0.0 : 1.0;

    return shadow;
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




