#version 330 core
layout (lines) in; // Odbiera linię (2 wierzchołki)
layout (line_strip, max_vertices = 2) out; // Wypuszcza linię

in VS_OUT {
    vec3 world_pos;
} gs_in[];

out float line_steepness;

uniform mat4 projection;
uniform mat4 view;

void main() {
    // 1. Obliczamy stromiznę dla całego segmentu linii
    vec2 p0 = gs_in[0].world_pos.xz;
    float h0 = gs_in[0].world_pos.y;

    vec2 p1 = gs_in[1].world_pos.xz;
    float h1 = gs_in[1].world_pos.y;

    float rise = abs(h1-h0);             // Różnica wysokości
    float run = length(p1-p0);         // Odległość w poziomie
    
    // Zabezpieczenie przed dzieleniem przez zero
    float steepness = rise / max(run, 0.0001); 

    // 2. Emitujemy oba wierzchołki z tą samą wartością steepness
    for(int i = 0; i < 2; i++) {
        gl_Position = projection * view * vec4(gs_in[i].world_pos, 1.0);
        
        line_steepness = steepness;
        
        EmitVertex();
    }
    
    EndPrimitive();
}