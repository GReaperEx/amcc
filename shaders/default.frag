#version 330 core

in vec2 fragUVs;
in vec3 fragNml;

out vec3 color;

uniform sampler2D mainTexture;

void main()
{
    color = texture(mainTexture, fragUVs).rgb*clamp(dot(fragNml, vec3(0.0, 1.0, 0.0)), 0.5, 1.0);
}
