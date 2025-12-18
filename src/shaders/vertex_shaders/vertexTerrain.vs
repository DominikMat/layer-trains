#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
  
out vec2 TexCoord;
out vec3 v_worldPos;

uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D heightmap;
uniform sampler2D built_path_mask;
uniform bool heightmap_enabled;
uniform float heightmap_scale;
uniform int heightmap_resolution_x = 1024;
uniform int heightmap_resolution_y = 1024;

void main()
{
    vec3 position = aPos;
    
    if (heightmap_enabled) {    
        // handle boundary
        float heightmap_step_x = 1.f / heightmap_resolution_x;
        float heightmap_step_y = 1.f / heightmap_resolution_y;
        if (aTexCoord.x < heightmap_step_x || aTexCoord.x > 1.f - heightmap_step_x
            || aTexCoord.y < heightmap_step_y || aTexCoord.y > 1.f - heightmap_step_y) {
            position.z = 0.f;
        } else {
            position.z = texture(heightmap, aTexCoord).x * heightmap_scale;
        }

        /* apply height from built road mask if needed */
        float local_road = texture(built_path_mask, aTexCoord).r;
        if (local_road > 0.001f) {
            position.z = local_road;
        }     
    } 

    v_worldPos = (transform * vec4(position, 1.0)).xyz;
    gl_Position = projection * view * vec4(v_worldPos, 1.0f);
    TexCoord = aTexCoord;
}