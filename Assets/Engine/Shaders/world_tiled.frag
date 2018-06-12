#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 worldPosition;

layout(location = 0) out vec4 outColor;

vec4 checker3D(vec3 texc, vec4 color0, vec4 color1)
{
  if ((int(floor(texc.x) + floor(texc.y) + floor(texc.z)) & 1) == 0)
    return color0;
  else
    return color1;
}

void main() {
    outColor = checker3D(worldPosition.xyz/2.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(0.2f, 0.2f, 0.2f, 1.0f));
}