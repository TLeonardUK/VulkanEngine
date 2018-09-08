layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord1;

layout(location = 0) out VertexOutputData {
    vec2 texCoord;
} outVertData;

void main() 
{
	outVertData.texCoord = inTexCoord1;
 
	gl_Position = vec4(inPosition, 0.0, 1.0);
}