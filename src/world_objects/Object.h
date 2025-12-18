 #ifndef OBJECT_H
#define OBJECT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "settings/Utility.h"
#include "textures/Texture.h"
#include "shaders/ShaderManager.h"
#include "ColourData.h"

using namespace glm;

class Object
{
public:
    vec3 position, size, rotation = vec3(0.f);
    
    bool custom_shader = false, visible = true, uses_texture = false, has_parent = false;
    bool render_to_world_pos = true, is_screen_object = false, render_props_changed = true;
    bool new_child_added = false;
    
    mat4 global_transform_matrix, local_transform_matrix;
    vec4 colour = Colour::PINK, tint_colour = Colour::WHITE;
    float opacity = 1.f;
    
    Shader *shader = new DEFAULT_WORLD_SHADER;
    Object *parent;
    std::vector<Object*> children;
    
    Object(vec3 pos = vec3(0.0f), vec3 size = vec3(1.0f)) : position(pos), size(size) {}
    virtual ~Object() = default;
        
    // necessary functions
    virtual void render() = 0;
    virtual void construct() = 0;
    virtual void configure_render_properties() { 
        if (custom_shader || !render_props_changed) return; 
        shader->setVec4("colour", vec4(colour.r,colour.g,colour.b, opacity));
        shader->setVec4("tint_colour", vec4(tint_colour.r,tint_colour.g,tint_colour.b, opacity));
        shader->setBool("useTexture", uses_texture);    
        render_props_changed = false;
    }
    virtual void disable_render_properties() {}
    virtual void initialize_shader_properties() {}

    // position
    void move(vec3 v){ this->position += v; }
    void set_position(vec3 p){ this->position = p; }

    // rotations
    void rotate(vec3 rt_xyz){ this->rotation += rt_xyz; }
    void set_rotation(vec3 rt_xyz){ this->rotation = rt_xyz; }

    // scaling
    void scale(vec3 s){ this->size *= s; }
    void scale(float s){ this->size *= vec3(s,s,s); }
    void set_size(vec3 s){ this->size = s; }
    void set_size(float s){ this->size = vec3(s,s,s); }

    // transform calculations
    virtual void calculate_local_transform() {
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
    virtual void set_visible(bool is_visible) { visible = is_visible; }
    virtual void set_parent(Object *parent) { if (!parent) { return; } has_parent = true; this->parent = parent; parent->add_child(this); }
    void add_child(Object *child) { children.push_back(child); new_child_added = true; }
    std::vector<Object*> get_children() { return children; }
    void set_transparency (float alpha) { opacity = glm::clamp(alpha, 0.f, 1.f); render_props_changed = true;  }
    void set_colour (vec3 new_colour) { colour = vec4(new_colour, 1.f); render_props_changed = true;  }
    void set_colour (vec4 new_colour) { colour = new_colour; if (new_colour.a !=1.f) { opacity = new_colour.a; render_props_changed = true;  } }
    void set_tint_colour (vec3 new_colour) { tint_colour = vec4(new_colour, 1.f); render_props_changed = true;  }
    void set_tint_colour (vec4 new_colour) { tint_colour = new_colour; if (new_colour.a !=1.f) { opacity = new_colour.a*colour.a; render_props_changed = true; } }
    virtual void set_texture (Texture *tex) { uses_texture = render_props_changed = true; shader->setTexture("Texture", tex); }
    virtual void set_shader (Shader *s) { shader = s; render_props_changed = true; custom_shader = true; }
    void set_screenspace() { is_screen_object = true; }
    void enable_shader() { shader->use(); }
    virtual void update_transform() { shader->setMatrix("transform", global_transform_matrix); }
    virtual int get_id() { return -1; }
};

#endif // OBJECT_H