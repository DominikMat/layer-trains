#version 330 core
out vec4 FragColor;

uniform vec3 colour;

void main()
{
    FragColor = colour;
}
