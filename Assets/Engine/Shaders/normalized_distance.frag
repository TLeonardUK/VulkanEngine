layout(set = 0, binding = 0) uniform MeshPropertiesBlock {
    mat4 model;
} meshProperties;

layout(set = 1, binding = 0) uniform ViewPropertiesBlock{
    mat4 view;
    mat4 proj;
    vec4 distanceOrigin;
	float maxDistance;
} viewProperties;

layout(location = 0) in vec4 worldPosition;

void main() 
{
	float distance = length(worldPosition.xyz - viewProperties.distanceOrigin.xyz);
	distance /= viewProperties.maxDistance;
	gl_FragDepth = distance;
} 