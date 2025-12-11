#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
  
out vec2 TexCoord;
out vec3 v_worldPos;

uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D heightmap;
uniform bool heightmap_enabled;
uniform float heightmap_scale;
uniform int heightmap_resolution_x = 1024;
uniform int heightmap_resolution_y = 1024;

void main()
{
    vec3 position = aPos;
    
    if (heightmap_enabled) { 
        vec2 texCoord = aTexCoord;
        
        // handle boundary
        float heightmap_step_x = 1.f / heightmap_resolution_x;
        float heightmap_step_y = 1.f / heightmap_resolution_y;
        if (texCoord.x < heightmap_step_x || texCoord.x > 1.f - heightmap_step_x
            || texCoord.y < heightmap_step_y || texCoord.y > 1.f - heightmap_step_y) {
            position.z = 0.f;
        } else {
            position.z = texture(heightmap, texCoord).x * heightmap_scale;
        }
    } 

    v_worldPos = (transform * vec4(position, 1.0)).xyz;
    gl_Position = projection * view * vec4(v_worldPos, 1.0f);
    TexCoord = aTexCoord;
}