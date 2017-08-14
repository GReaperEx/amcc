#version 330 core

layout(location = 0) in vec3 vtxPos;
layout(location = 1) in vec2 vtxUVs;
layout(location = 2) in vec3 vtxNml;

out vec2 fragUVs;
out vec3 fragNml;

uniform mat4 MVP;

void main()
{
    fragUVs = vec2(vtxUVs.x, 1.0 - vtxUVs.y);
    fragNml = vtxNml;
    gl_Position = MVP*vec4(vtxPos, 1.0);
}
