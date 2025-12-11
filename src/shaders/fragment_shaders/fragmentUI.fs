#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D image;
uniform vec4 colour;
uniform vec4 tint_colour;
uniform bool useTexture;
uniform bool isText;

void main()
{
    if (isText) {
        vec4 sampled = vec4(1.0, 1.0, 1.0, texture(image, TexCoord).r);
        FragColor = colour * tint_colour * sampled;
    } 
    else if (useTexture) {
        FragColor = texture(image, TexCoord) * tint_colour;
    } 
    else {
        FragColor = colour * tint_colour;
    }
    
    if (FragColor.a < 0.01) discard;
}