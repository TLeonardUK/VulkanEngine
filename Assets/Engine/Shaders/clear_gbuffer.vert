layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord1;

layout(location = 0) out vec2 outFragPos;

void main() 
{
    outFragPos = inPosition;
    gl_Position = vec4(inPosition, 0.0, 1.0);
}