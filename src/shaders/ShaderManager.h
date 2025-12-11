#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include "shaders/Shader.h"
#include "shaders/ShaderData.h"
#include "textures/TextureData.h"
#include "Heightmap.h"
#include <glm/glm.hpp>

#define DEFAULT_WORLD_SHADER Shader(VERTEX_BASIC_PATH, FRAGMENT_BASIC_PATH)
#define WORLD_UI_SHADER Shader(VERTEX_BASIC_PATH, FRAGMENT_UI_PATH)
#define SCREEN_UI_SHADER Shader(VERTEX_UI_PATH, FRAGMENT_UI_PATH)

class ShaderManager {
public:
    // static Shader& get_default_world_shader(Texture *tex = &TEXTURE_MISSING) {
    //     static Shader *shader = new Shader(VERTEX_BASIC_PATH, FRAGMENT_BASIC_PATH);
    //     shader->use();
    //     shader->addTexture( tex ); shader->setInt("texture", shader->get_last_loaded_tex_slot());
    //     return *shader;
    // }
    // static Shader& get_default_world_shader() {
    //     static Shader shader = Shader(VERTEX_BASIC_PATH, FRAGMENT_BASIC_PATH);
    //     return shader;
    // }
    
    static Shader& get_terrain_shader(Texture *heightmap_tex, float heightmap_scale, Texture *texture = &TEXTURE_MISSING) {
        static Shader shader = Shader(VERTEX_TERRAIN_PATH, FRAGMENT_TERRAIN_PATH);
        shader.use();
        shader.addTexture(texture); shader.setInt("texture", shader.get_last_loaded_tex_slot());
        shader.addTexture(heightmap_tex); shader.setInt("heightmap", shader.get_last_loaded_tex_slot());
        return shader;
    }

    static Shader& get_line_shader(glm::vec3 line_colour) {
        Shader shader = Shader(VERTEX_LINE_PATH, FRAGMENT_LINE_PATH);
        shader.use();
        shader.setVec3("lineColor", line_colour );
        return shader;
    }
    
    // static Shader& get_screen_ui_shader() {
    //     static Shader shader = Shader(VERTEX_UI_PATH, FRAGMENT_UI_PATH);
    //     return shader;
    // }
    // static Shader& get_world_ui_shader() {
    //     static Shader shader = ;
    //     return shader;
    // }
};

#endif // SHADERMANAGER_H
