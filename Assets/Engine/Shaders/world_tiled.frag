#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 worldPosition;

layout(location = 0) out vec4 gbuffer0;
layout(location = 1) out vec4 gbuffer1;
layout(location = 2) out vec4 gbuffer2;

vec4 checker3D(vec3 texc, vec4 color0, vec4 color1)
{
  if ((int(floor(texc.x) + floor(texc.y) + floor(texc.z)) & 1) == 0)
    return color0;
  else
    return color1;
}

void main() 
{
    gbuffer0.rgba = checker3D(worldPosition.xyz/2.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(0.2f, 0.2f, 0.2f, 1.0f));
    gbuffer1.rgba = vec4(1.0, 0.0, 1.0, 1.0);
    gbuffer2.rgba = vec4(1.0, 1.0, 0.0, 1.0);
}