#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include "shaders/Shader.h"
#include "shaders/ShaderData.h"
#include "textures/TextureData.h"
#include "Heightmap.h"
#include <glm/glm.hpp>

class ShaderManager {
public:
    static Shader& get_default_world_shader(Texture tex = TEXTURE_MISSING) {
        static Shader shader = Shader(VERTEX_BASIC_PATH, FRAGMENT_BASIC_PATH);
        shader.setTexture( tex );
        return shader;
    }

    static Shader& get_terrain_shader(Texture heightmap_tex, float heightmap_scale, Texture texture = TEXTURE_MISSING) {
        static Shader shader = Shader(VERTEX_TERRAIN_PATH, FRAGMENT_TERRAIN_PATH);
        shader.setTexture( texture );
        shader.setHeightmap( heightmap_tex, heightmap_scale );
        return shader;
    }

    static Shader& get_line_shader(glm::vec3 line_colour) {
        static Shader shader = Shader(VERTEX_LINE_PATH, FRAGMENT_LINE_PATH);
        shader.setVec3("lineColor", line_colour );
        return shader;
    }
    
    static Shader& get_screen_ui_shader() {
        static Shader shader = Shader(VERTEX_UI_PATH, FRAGMENT_UI_PATH);
        return shader;
    }
    static Shader& get_world_ui_shader() {
        static Shader shader = Shader(VERTEX_BASIC_PATH, FRAGMENT_UI_PATH);
        return shader;
    }

    static Shader& get_basic_colour_shader() {
        static Shader shader = Shader(VERTEX_BASIC_PATH, FRAGMENT_SIMPLE_COLOUR_PATH);
        return shader;
    }
};

#endif // SHADERMANAGER_H
