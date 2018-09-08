layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in struct {
    vec4 Color;
    vec2 UV;
} In;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = In.Color * texture(texSampler, In.UV.st);
}