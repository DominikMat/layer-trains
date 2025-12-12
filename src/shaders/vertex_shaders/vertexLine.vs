#version 330 core
layout (location = 0) in vec3 aPos;

out VS_OUT {
    vec3 world_pos;
} vs_out;

uniform sampler2D heightmap;
uniform float heightmap_scale;
uniform mat4 transform;
uniform float terrain_offset_distance; 
uniform int heightmap_resolution_x; 
uniform int heightmap_resolution_y; 

vec2 local_to_uv (vec2 local) { return vec2(local.x + 0.5, local.y + 0.5); }

vec3 calculate_terrain_normal(vec2 uv) {
    float step_x = 1.f / heightmap_resolution_x;
    float step_y = 1.f / heightmap_resolution_y;
    float hL = texture(heightmap, uv - vec2(step_x, 0)).r * heightmap_scale; // Lewo
    float hR = texture(heightmap, uv + vec2(step_x, 0)).r * heightmap_scale; // Prawo
    float hD = texture(heightmap, uv - vec2(0, step_y)).r * heightmap_scale; // Dół
    float hU = texture(heightmap, uv + vec2(0, step_y)).r * heightmap_scale; // Góra

    vec3 normal = normalize(vec3(
        hL - hR,
        (step_x+step_y) * heightmap_scale * 20.0,
        hD - hU
    ));
    return normal;
}

void main()
{
    vec2 local_pos = aPos.xy;
    vec2 uv = local_to_uv(local_pos);
    float local_height = texture(heightmap, uv).r * heightmap_scale;
    
    vec3 normal = calculate_terrain_normal(uv);

    vec4 worldPosition = transform * vec4(aPos.xy, local_height, 1.0);
    worldPosition.xyz += normal * terrain_offset_distance;
    vs_out.world_pos = worldPosition.xyz;
}