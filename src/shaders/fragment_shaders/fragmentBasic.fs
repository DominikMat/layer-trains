#version 330 core

out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D Texture;
uniform bool useTexture = false;

uniform vec4 colour;


void main()
{
    if (useTexture) {
        FragColor = texture(Texture, TexCoord);
    }
    else {
        FragColor = colour;
    }
}
