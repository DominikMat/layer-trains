#version 330 core

in vec2 TexCoord;
in vec3 v_worldPos;
out vec4 FragColor;

// --- Mouse position drawing ---
uniform bool u_renderWorldPos;
uniform sampler2D world_pos_texture; // Tekstura z Pass 1
uniform vec2 u_mouseCoords;        // Mysz w [0, 1]
uniform float u_circleOuterRadius; // np. 10.0
uniform float u_circleInnerRadius; // np. 8.0
uniform sampler2D Texture;

void main(){
    if (u_renderWorldPos) {
        FragColor = vec4(v_worldPos, 1.0);
        return;
    }

    vec3 finalColor = texture(Texture, TexCoord).rgb;

    // --- Logika rysowania cursoru (Decal) ---
    vec3 mouseWorldPos = texture(world_pos_texture, u_mouseCoords).rgb;
    float dist = distance(v_worldPos, mouseWorldPos);
    float circleAlpha = dist > u_circleInnerRadius && dist < u_circleOuterRadius ? 1.0 :0.0;
    vec3 circleColor = vec3(0.0, 0.2, 1.0); // Niebieski

    // 4. Mieszaj kolor terenu z kolorem kółka
    if (mouseWorldPos.x != 0.0 || mouseWorldPos.y != 0.0 || mouseWorldPos.z != 0.0) {
         finalColor = mix(finalColor, circleColor, circleAlpha);
    }

    FragColor = vec4(finalColor, 1.0);
}
