layout(set = 0, binding = 0) uniform GlobalPropertiesBlock {
    mat4 view;
    mat4 proj;
} globalProperties;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main() 
{
	outColor = inColor;

    gl_Position = globalProperties.proj * globalProperties.view * vec4(inPosition, 1.0);
}