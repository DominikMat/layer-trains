 #ifndef OBJECT_H
#define OBJECT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "settings/Utility.h"
#include "textures/Texture.h"
#include "shaders/ShaderManager.h"
#include "ColourData.h"

using namespace glm;

class Object
{
public:
    vec3 position;
    vec3 size;
    vec3 rotation = vec3(0.f);
    bool visible = true;
    Shader *shader;

    vec4 colour = Colour::PINK, tint_colour = Colour::WHITE;
    float opacity = 1.f;
    bool uses_texture = false;

    bool has_parent = false;
    Object *parent;

    bool render_to_world_pos = true;

    mat4 global_transform_matrix, local_transform_matrix;

    bool is_screen_object = false;

    virtual ~Object() = default;

    Object(vec3 pos = vec3(0.0f), vec3 size = vec3(1.0f))
        : position(pos), size(size) {

        shader = new DEFAULT_WORLD_SHADER;
    }

    // necessary functions
    virtual void render() = 0;
    virtual void construct() = 0;
    virtual void configure_render_properties() {
        shader->setVec4("colour", vec4(colour.r,colour.g,colour.b, opacity));
        shader->setVec4("tint_colour", vec4(tint_colour.r,tint_colour.g,tint_colour.b, opacity));
        shader->setBool("useTexture", uses_texture);
    };
    virtual void disable_render_properties() {};

    // position
    void move(vec3 v){ this->position += v; }
    void set_position(vec3 p){ this->position = p; }

    // rotations
    void rotate(vec3 rt_xyz){ this->rotation += rt_xyz; }
    void set_rotation(vec3 rt_xyz){ this->rotation = rt_xyz; }

    // scaling
    void scale(vec3 s){ this->size *= s; }
    void scale(float s){ this->size *= vec3(s,s,s); }
    void set_size(vec3 s){    this->size = s; }
    void set_size(float s){ this->size = vec3(s,s,s); }

    // transform calculations
    void calculate_local_transform() {
        local_transform_matrix = mat4(1.0f);
        local_transform_matrix = glm::translate(local_transform_matrix, this->position);
        local_transform_matrix = glm::rotate(local_transform_matrix, glm::radians(this->rotation.x), V3_X);
        local_transform_matrix = glm::rotate(local_transform_matrix, glm::radians(this->rotation.y), V3_Y);
        local_transform_matrix = glm::rotate(local_transform_matrix, glm::radians(this->rotation.z), V3_Z);
        local_transform_matrix = glm::scale(local_transform_matrix, this->size);
    }
    virtual void calculate_transform_matrix() {
        calculate_local_transform();
 
        if (has_parent && !is_screen_object) { // screen objects use UIObject class to position themselves to parent
            global_transform_matrix = parent->get_transform() * local_transform_matrix;
        } else {
            global_transform_matrix = local_transform_matrix;
        }
    }
    
    // basic fucntions
    mat4 get_transform() {    return global_transform_matrix; }
    void set_visible(bool is_visible) { visible = is_visible; }
    virtual void set_parent(Object *parent) { has_parent = true; this->parent = parent; }
    void set_transparency (float alpha) { opacity = glm::clamp(alpha, 0.f, 1.f); }
    void set_colour (vec3 new_colour) { colour = vec4(new_colour, 1.f); }
    void set_colour (vec4 new_colour) { colour = new_colour; if (new_colour.a !=1.f) { opacity = new_colour.a; } }
    void set_tint_colour (vec3 new_colour) { tint_colour = vec4(new_colour, 1.f); }
    void set_tint_colour (vec4 new_colour) { tint_colour = new_colour; if (new_colour.a !=1.f) { opacity = new_colour.a*colour.a; } }
    void set_texture (Texture *tex) { uses_texture = true; shader->addTexture(tex); shader->use(); shader->setInt("texture", shader->get_last_loaded_tex_slot()); }
    virtual void set_shader (Shader *s) { shader = s; }
    void set_screenspace() { is_screen_object = true; }
    void enable_shader() { shader->use(); }
    virtual void update_transform() { shader->setMatrix("transform", global_transform_matrix); }
    virtual int get_id() { return -1; }
};

#endif // OBJECT_H