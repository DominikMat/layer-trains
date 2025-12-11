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

 // --- Terrain height data ---
uniform float terrain_max_height;
uniform float terrain_min_height;
uniform int heightmap_resolution_x;
uniform int heightmap_resolution_y;
uniform int terrain_boundrary_pixel_width;
uniform vec3 terrain_boundary_colour;

// --- Terrain contour lines ---
uniform float iso_line_spacing;
uniform float iso_line_thickness;
uniform float camera_zoom_level;
uniform vec4 iso_line_colour;    

// --- Terrain colour pallete ---
uniform sampler2D elevation_gradient;  
uniform sampler2D steepness_gradient;    
uniform sampler2D water_gradient;    
uniform float elevation_gradient_max_height;
uniform float elevation_gradient_strength;
uniform float steepness_scale;

// --- Terrain textures ---
uniform sampler2D heightmap;
uniform sampler2D terrain_area_data;

// --- Terrain water parameters ---
uniform vec3 water_inside_colour;    
uniform vec3 water_outline_colour;    
uniform float water_level_height;

// --- Terrain snow parameters ---
uniform vec4 snow_colour;    
uniform float snow_level_height;    
uniform float snow_falloff_range;
uniform float snow_max_steepness;

/* Colours */
const vec3 cursor_colour = vec3(0.0, 0.2, 1.0); // blue

/* Functions */
float gridLayer(float height, float spacing, float thickness, float zoom_fade_start);

void main(){
    /* Render to depth buffer */
    if (u_renderWorldPos) {
        FragColor = vec4(v_worldPos, 1.0);
        return;
    }

    /* Check if height map edge -> boundary colour */
    float texel_x = 1.f / heightmap_resolution_x;
    float texel_y = 1.f / heightmap_resolution_y;
    if (TexCoord.x < terrain_boundrary_pixel_width*texel_x || TexCoord.x > 1.f-terrain_boundrary_pixel_width*texel_x
        || TexCoord.y < terrain_boundrary_pixel_width*texel_y || TexCoord.y > 1.f-terrain_boundrary_pixel_width*texel_y){
        
        FragColor = vec4(terrain_boundary_colour, 1.f);
        return;
    }  

    /* Get pixel height, elevation, and steepness (finite diff) */
    float local_height = texture(heightmap, TexCoord).r;
    float current_elevation = terrain_min_height + local_height * (terrain_max_height-terrain_min_height);
    const int px_offset = 2;
    float dx = textureOffset(heightmap, TexCoord, ivec2(px_offset,0)).r - textureOffset(heightmap, TexCoord, ivec2(-px_offset,0)).r;
    float dy = textureOffset(heightmap, TexCoord, ivec2(0,px_offset)).r - textureOffset(heightmap, TexCoord, ivec2(0,-px_offset)).r;
    float steepness = clamp(length(vec2(dx,dy)) * 0.5 * steepness_scale, 0.0, 1.0);
    vec3 colour = vec3(0.0,0.0,0.0);

    /* Check pixel area */
    bool sea_pixel = current_elevation < water_level_height;
    bool dry_pixel = !sea_pixel;
    bool snow_pixel = current_elevation > snow_level_height;

    /* apply water colour */
    if (sea_pixel){
        float dist_from_shore = (water_level_height-current_elevation)/water_level_height;
        colour = texture(water_gradient, vec2(dist_from_shore,0.5)).rgb;
    } 
    
    /* apply terrain colour */
    if (dry_pixel) {
        float elevation_t = clamp(current_elevation/elevation_gradient_max_height, 0.0, 1.0);
        vec3 elevation_colour = texture(elevation_gradient, vec2(elevation_t, 0.5)).rgb;
        vec3 steepness_colour = texture(steepness_gradient, vec2(steepness, 0.5)).rgb;
        colour = mix(steepness_colour, elevation_colour, elevation_gradient_strength); //elevation_gradient_strength);
    }

    /* add snow colour */
    if (snow_pixel) {
        float height_above = current_elevation - snow_level_height;
        float above_snow_level_mult = height_above / snow_falloff_range;
        float falloff_ratio = clamp(above_snow_level_mult*above_snow_level_mult, 0.0f, 1.0f);
        float allowed_steepness = mix(snow_max_steepness, 1.0f, falloff_ratio);
        
        if (steepness < allowed_steepness) {
            float snow_transition = clamp(height_above / 50.0f, 0.0f, 1.0f); // Smooth transition at the very bottom edge of snow line
            colour = mix(colour, snow_colour.rgb, snow_transition*snow_colour.a);
        }
    }

    /* Draw iso lines */
    if (!sea_pixel) {
        float base_spacing = 10.0; 
        float minor = gridLayer(current_elevation, base_spacing * 1.0, 1.5, 20.0);
        float medium = gridLayer(current_elevation, base_spacing * 5.0, 2.25, 13.0);
        float major = gridLayer(current_elevation, base_spacing * 20.0, 3.0, 0.0);
        float total_line = max(major, max(medium, minor));
        
        if (steepness < 0.001) total_line = 0.0;
        colour = mix(colour, iso_line_colour.rgb, total_line*iso_line_colour.a);
    }

    /* Draw mouse cursor on Terrain */
    vec3 mouseWorldPos = texture(world_pos_texture, u_mouseCoords).rgb;
    float dist = distance(v_worldPos, mouseWorldPos);
    colour = dist > u_circleInnerRadius && dist < u_circleOuterRadius ? cursor_colour : colour;

    /* Final Colour */
    FragColor = vec4(colour, 1.0);
}

float gridLayer(float height, float spacing, float thickness, float zoom_fade_start) {
    float scaledHeight = height / spacing;
    float dist = abs(fract(scaledHeight - 0.5) - 0.5) * spacing;
    float screen_width = fwidth(height); 
    float line_val = 1.0 - smoothstep(0.0, screen_width * thickness, dist);
    float spacing_on_screen = spacing / screen_width; 
    float fade = clamp((spacing_on_screen - zoom_fade_start) / 10.0, 0.0, 1.0);
    return line_val * fade;
}
