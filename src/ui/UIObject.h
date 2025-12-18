#ifndef UI_OBJECT_H
#define UI_OBJECT_H

#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include "Object.h"

enum class UIAnchor {
    TOP_LEFT,    TOP_CENTER,    TOP_RIGHT,
    MIDDLE_LEFT, CENTER,        MIDDLE_RIGHT,
    BOTTOM_LEFT, BOTTOM_CENTER, BOTTOM_RIGHT,
};

class Button;

class UIObject : public Object {
protected:
    Texture *active_texture = nullptr;
    std::vector<UIObject*> ui_children;

public:
    UIAnchor anchor;
    glm::vec2 anchor_offset; // Offset from the anchor point in pixels

    float scr_width = SCR_WIDTH;
    float scr_height = SCR_HEIGHT;

    UIObject(glm::vec2 size, UIAnchor anchor = UIAnchor::CENTER, glm::vec2 anchor_offset = glm::vec2(0.f))
        : anchor(anchor), anchor_offset(anchor_offset), Object(get_object_anchored_position(anchor, anchor_offset),vec3(size,1.f))
    {}
    UIObject(glm::vec2 position, glm::vec2 size)
        : Object(glm::vec3(position,0.f), glm::vec3(size,1.f)), anchor(UIAnchor::BOTTOM_LEFT), anchor_offset(vec2(0.f))
    {}

    void set_anchor (UIAnchor anchor, vec2 anchor_offset) {
        this->anchor = anchor;
        this->anchor_offset = anchor_offset;
        position = get_object_anchored_position(anchor, anchor_offset);
        set_screenspace();
        resize_and_reposition();
    }

    virtual void set_parent(Object *parent) override {
        Object::set_parent(parent);
        if (is_screen_object){
            position = get_object_anchored_position(anchor, anchor_offset);
            resize_and_reposition();
        }
    }
    void set_ui_parent(UIObject *parent) {
        set_parent(parent);
        parent->add_ui_child(this);
    }

    vec2 get_anchor_position(UIAnchor anchor) {
        float relative_size_x = has_parent ? parent->size.x : scr_width;
        float relative_size_y = has_parent ? parent->size.y : scr_height;

        vec2 anchor_position_offset = vec2(0);
        switch (anchor) {  
            case UIAnchor::TOP_LEFT:      anchor_position_offset = vec2(size.x/2+-relative_size_x/2, -size.y/2+relative_size_y/2); break;
            case UIAnchor::TOP_CENTER:    anchor_position_offset = vec2(0, -size.y/2+relative_size_y/2); break;
            case UIAnchor::TOP_RIGHT:     anchor_position_offset = vec2(-size.x/2+relative_size_x/2, -size.y/2+relative_size_y/2); break;
            case UIAnchor::MIDDLE_LEFT:   anchor_position_offset = vec2(size.x/2+-relative_size_x/2, 0); break;
            case UIAnchor::CENTER:        anchor_position_offset = vec2(0, 0); break;
            case UIAnchor::MIDDLE_RIGHT:  anchor_position_offset = vec2(-size.x/2+relative_size_x/2, 0); break;
            case UIAnchor::BOTTOM_LEFT:   anchor_position_offset = vec2(size.x/2+-relative_size_x/2, size.y/2+-relative_size_y/2); break;
            case UIAnchor::BOTTOM_CENTER: anchor_position_offset = vec2(0, size.y/2+-relative_size_y/2); break;
            case UIAnchor::BOTTOM_RIGHT:  anchor_position_offset = vec2(-size.x/2+relative_size_x/2, size.y/2+-relative_size_y/2); break;
        }
        return anchor_position_offset + (has_parent ? parent->position : vec2(scr_width/2,scr_height/2));
    }
    vec3 get_object_anchored_position(UIAnchor anchor, vec2 anchor_offset) {
        return vec3(get_anchor_position(anchor) + anchor_offset,0.f);
    }

    void update_screen_size(float w, float h) {
        scr_width = w; scr_height = h;
        recalculate_ui_position(); 
    }

    void recalculate_ui_position() {
        position = get_object_anchored_position(anchor, anchor_offset);
    }

    virtual void resize_and_reposition() {
        recalculate_ui_position();
        for(auto* c : ui_children) {
            c->recalculate_ui_position();
        }
    }

    virtual Button* get_button() {
        return nullptr;
    }
    std::vector<UIObject*> get_ui_children() {
        return ui_children;
    }
    void add_ui_child(UIObject* child) {
        ui_children.push_back(child); new_child_added = true;
    }

    virtual void configure_render_properties() override { 
        //if (!render_props_changed) return; 
        shader->setVec4("colour", vec4(colour.r,colour.g,colour.b, opacity));
        shader->setVec4("tint_colour", tint_colour);
        shader->setBool("useTexture", uses_texture);    
        render_props_changed = false;
    }

    virtual bool is_mouse_over(vec2 mouse_pixel_pos) {
        if (!visible) return false;
        
        float left   = position.x - (size.x / 2.0f);
        float right  = position.x + (size.x / 2.0f);
        float top    = position.y + (size.y / 2.0f);
        float bottom = position.y - (size.y / 2.0f);
        
        return (mouse_pixel_pos.x >= left && mouse_pixel_pos.x <= right && mouse_pixel_pos.y >= bottom && mouse_pixel_pos.y <= top);
    }

    void initialize_shader_properties() override {
        shader->setVec4("colour", vec4(vec3(colour),opacity));
        shader->setVec4("tint_colour", tint_colour);
        shader->setBool("useTexture", uses_texture);    
    }

    void set_texture(Texture *tex) override { 
        active_texture = tex;
        uses_texture = true;
        shader->setTexture("image", tex); 
        render_props_changed = true; 
    }

    void set_shader(Shader *s) override {
        Object::set_shader(s);
        if (active_texture != nullptr) set_texture(active_texture);
    }

    void set_visible(bool state) override {
        Object::set_visible(state);
        for(auto* c : ui_children) {
            c->set_visible(state);
        }
    }

    ~UIObject() override {
        for (auto* c : ui_children) delete c;
        delete this;
    }
};

#endif