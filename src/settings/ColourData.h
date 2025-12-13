#ifndef COLOURDATA_H
#define COLOURDATA_H

#include <glm/glm.hpp>

// Używamy namespace Colour, aby uniknąć kolizji nazw.
// Wszystkie kolory są w formacie RGBA (czerwony, zielony, niebieski, alfa).
// Wartości są w zakresie [0.0, 1.0].
namespace Colour {
    using glm::vec4;

    // --- Podstawowe kolory ---
    const vec4 WHITE      = vec4(1.0f, 1.0f, 1.0f, 1.0f); // Biały
    const vec4 BLACK      = vec4(0.0f, 0.0f, 0.0f, 1.0f); // Czarny
    const vec4 RED        = vec4(1.0f, 0.0f, 0.0f, 1.0f); // Czerwony
    const vec4 GREEN      = vec4(0.0f, 1.0f, 0.0f, 1.0f); // Zielony
    const vec4 BLUE       = vec4(0.0f, 0.0f, 1.0f, 1.0f); // Niebieski
    const vec4 YELLOW     = vec4(1.0f, 1.0f, 0.0f, 1.0f); // Żółty (Red + Green)
    const vec4 CYAN       = vec4(0.0f, 1.0f, 1.0f, 1.0f); // Cyjan (Green + Blue)
    const vec4 MAGENTA    = vec4(1.0f, 0.0f, 1.0f, 1.0f); // Magenta (Red + Blue)
    const vec4 TRANSPARENT= vec4(0.0f, 0.0f, 0.0f, 0.0f); // W pełni przezroczysty

    // --- Odcienie szarości ---
    const vec4 GREY       = vec4(0.5f, 0.5f, 0.5f, 1.0f); // Szary
    const vec4 LIGHT_GREY = vec4(0.75f, 0.75f, 0.75f, 1.0f); // Jasnoszary
    const vec4 DARK_GREY  = vec4(0.25f, 0.25f, 0.25f, 1.0f); // Ciemnoszary

    // --- Kolory UI / Webowe ---
    const vec4 ORANGE     = vec4(1.0f, 0.5f, 0.0f, 1.0f); // Pomarańczowy
    const vec4 PURPLE     = vec4(0.5f, 0.0f, 0.5f, 1.0f); // Fioletowy
    const vec4 PINK       = vec4(1.0f, 0.4f, 0.7f, 1.0f); // Różowy
    const vec4 GOLD       = vec4(1.0f, 0.84f, 0.0f, 1.0f); // Złoty
    const vec4 SKY_BLUE   = vec4(0.53f, 0.81f, 0.98f, 1.0f); // Błękitny

    // --- Dodatkowe ---
    const vec4 DARK_BROWN = vec4(46/255.f, 26/255.f, 0, 1.f); // Dark brown 'rgba(46, 26, 0, 1)'
    
    // --- Palette ---
    const vec4 TERRAIN_SIDE_COLOUR = vec4(43/255.f, 37/255.f, 29/255.f, 1.f); // greyish brown 'rgba(43, 37, 29, 1)'
    const vec4 SNOW_COLOUR = vec4(1.f,1.f,1.f,0.75f); // greyish brown 'rgba(180, 180, 180, 1)'
    const vec4 DEFAULT_RENDER_BACKGROUND = vec4(132.f/255.f, 178.f/255.f, 179.f/255.f, 1.0f);
}

#endif // COLOURDATA_H