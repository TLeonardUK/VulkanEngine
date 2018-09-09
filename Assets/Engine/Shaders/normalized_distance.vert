layout(set = 0, binding = 0) uniform MeshPropertiesBlock {
    mat4 model;
} meshProperties;

layout(set = 1, binding = 0) uniform ViewPropertiesBlock{
    mat4 view;
    mat4 proj;
    vec4 distanceOrigin;
	float maxDistance;
} viewProperties;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec4 worldPosition;

void main() 
{
	worldPosition = meshProperties.model * vec4(inPosition, 1.0);;
    gl_Position = viewProperties.proj * viewProperties.view * worldPosition;
}