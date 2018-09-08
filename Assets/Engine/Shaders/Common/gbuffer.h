struct GBufferTexel
{
	vec3 albedo;
	uint flags;
	
	vec3 worldNormal;
	vec3 worldPosition;	
};


#define writeGBuffer(texel)												\
{																		\
    gbuffer0.rgba = vec4(texel.albedo.xyz, texel.flags / 255.0);		\
    gbuffer1.rgba = vec4(texel.worldNormal.xyz, 1.0);					\
    gbuffer2.rgba = vec4(texel.worldPosition.xyz, 1.0);					\
}

#define readGBuffer(texel, uv) 											\
{ 																		\
	vec4 gbuffer0Encoded = texture(gbuffer0, uv); 						\
	vec4 gbuffer1Encoded = texture(gbuffer1, uv); 						\
	vec4 gbuffer2Encoded = texture(gbuffer2, uv); 						\
																		\
	texel.albedo = gbuffer0Encoded.rgb;									\
	texel.flags = uint(gbuffer0Encoded.a * 255.0f);;						\
	texel.worldNormal = gbuffer1Encoded.rgb;							\
	texel.worldPosition = gbuffer2Encoded.rgb;							\
}