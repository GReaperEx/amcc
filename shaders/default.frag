#version 330 core

in vec2 fragUVs;
in vec3 fragNml;

out vec3 color;

uniform sampler2D mainTexture;

void main()
{
    vec4 textureColor = texture(mainTexture, fragUVs);
    if (textureColor.a == 0.0) {
        discard;
    }
    color = textureColor.rgb*clamp(dot(fragNml, vec3(0.0, 1.0, 0.0)), 0.5, 1.0);
}
