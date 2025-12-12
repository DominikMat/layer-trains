#version 330 core
out vec4 FragColor;
in float line_steepness;

// uniform float steepness_scale;
// uniform sampler2D heightmap;
uniform float max_steepness_value;

uniform vec3 line_colour;
uniform vec3 max_steepness_colour;
uniform vec3 min_steepness_colour;
uniform bool show_steepness;

vec2 local_to_uv (vec2 local) { return vec2(local.x+0.5,local.y+0.5); }

void main()
{   
    if (show_steepness) {
        float steepness_t = clamp(abs(line_steepness)/max_steepness_value, 0.0, 1.0);
        vec3 steepness_colour = mix(min_steepness_colour, max_steepness_colour, steepness_t);
        FragColor = vec4(steepness_colour, 1.0 );
    } 
    else FragColor = vec4(line_colour, 1.0);
}
