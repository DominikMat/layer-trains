#version 330 core

out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D Texture;

void main()
{
    // vec4(1.0, 0.0, 1.0, 1.0) --> bright pink (magenta)
    FragColor = texture(Texture, TexCoord);
}
