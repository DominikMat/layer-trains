#ifndef ScreenObject_H
#define ScreenObject_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "settings/Utility.h"
#include "textures/Texture.h"
#include "shaders/ShaderManager.h"

using namespace glm;

class ScreenObject
{
public:
    // Common transformation and appearance properties for all ScreenObjects
    vec2 position;
    vec2 size;
    float rotation;
    bool visible, uses_custom_shader = false;
    Shader *shader;

    bool has_parent = false;
    ScreenObject *parent;

    mat4 global_transform_matrix, local_transform_matrix;

    virtual ~ScreenObject() = default;

    ScreenObject(vec2 pos, vec2 size)
        : position(pos), size(size), rotation(0.f), visible(true) {
    }

    virtual void construct() = 0;
    virtual void render() = 0;
    virtual void configure_render_properties(Shader *s = NULL) = 0;
    
    // position
    void move(vec2 v){ this->position += v; }
    void set_position(vec2 p){ this->position = p; }

    // rotations
    void rotate(float deg){ this->rotation += deg; }
    void set_rotation(float deg) { this->rotation = deg; }

    // modify scaling
    void scale(vec2 s){ this->size *= s; }
    void scale(float s){ this->size *= vec2(s,s); }
    void set_size(vec2 s){ this->size = s; }
    void set_size(float s){ this->size = vec2(s,s); }

    void calculate_local_transform() {
        local_transform_matrix = mat4(1.0f);
        local_transform_matrix = glm::translate(local_transform_matrix, vec3(this->position,0.f));
        local_transform_matrix = glm::rotate(local_transform_matrix, glm::radians(this->rotation), V3_Z);
        local_transform_matrix = glm::scale(local_transform_matrix, vec3(this->size,1.f));
    }

    void calculate_transform_matrix() {
        calculate_local_transform();

        if (has_parent) {
            global_transform_matrix = parent->get_transform() * local_transform_matrix;
        } else {
            global_transform_matrix = local_transform_matrix;
        }
    }

    mat4 get_transform() {
        return global_transform_matrix;
    }

    void set_visible(bool is_visible) {
        visible = is_visible;
    }

    void set_custom_shader(Shader *shader) {
        uses_custom_shader = true;
        this->shader = shader; 
    }
    void enable_shader() {
        if (uses_custom_shader && shader != NULL)
            shader->use();
    }

    void set_parent(ScreenObject *parent) {
        has_parent = true;
        this->parent = parent;
    }

};

#endif // ScreenObject_H