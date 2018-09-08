layout(set = 0, binding = 0) uniform MeshPropertiesBlock {
    mat4 model;
	int flags;
} meshProperties;

layout(set = 1, binding = 0) uniform GlobalPropertiesBlock {
    mat4 view;
    mat4 proj;
    vec3 camPosition;
} globalProperties;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord1;

layout(location = 0) out vec4 outWorldPosition;
layout(location = 1) out vec4 outWorldNormal;
layout(location = 2) out vec2 outTexCoord1;

void main() 
{
    outTexCoord1 = inTexCoord1;
    outWorldNormal = meshProperties.model * vec4(inNormal, 1.0);
    outWorldPosition = meshProperties.model * vec4(inPosition, 1.0);

    gl_Position = globalProperties.proj * globalProperties.view * meshProperties.model * vec4(inPosition, 1.0);
}