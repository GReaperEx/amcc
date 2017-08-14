#version 330 core

in vec2 fragUVs;

out vec3 color;

uniform sampler2D mainTexture;

void main()
{
    color = texture(mainTexture, fragUVs).rgb;
}
