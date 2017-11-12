#version 330 core

in vec2 fragUVs;
in float fragLight;

out vec3 color;

uniform sampler2D mainTexture;

void main()
{
    vec4 textureColor = texture(mainTexture, fragUVs);
    if (textureColor.a == 0.0) {
        discard;
    }
    color = textureColor.rgb*clamp(float(fragLight)+2.0f, 0.0f, 15.0f)*0.06666666f;
}
