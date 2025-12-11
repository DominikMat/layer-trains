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
        // FreeType przechowuje kształt litery tylko w kanale RED.
        // Pobieramy wartość R i używamy jej jako Alpha.
        vec4 sampled = vec4(1.0, 1.0, 1.0, texture(image, TexCoord).r);
        
        // Mieszamy kolor tekstu z kształtem litery (alpha)
        FragColor = colour * tint_colour * sampled;
    } 
    else if (useTexture) {
        // Zwykła tekstura (np. ikona) * tint koloru
        FragColor = texture(image, TexCoord) * tint_colour;
    } 
    else {
        // Zwykły panel (tylko kolor)
        FragColor = colour * tint_colour;
    }
    
    // Odrzuć w pełni przezroczyste piksele (opcjonalne)
    if (FragColor.a < 0.01)
        discard;
}