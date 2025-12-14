#ifndef TextButton_H
#define TextButton_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

#include "Button.h"
#include "UIText.h"
#include "Panel.h"

using namespace glm;

class TextButton : public Button
{
protected:
    const vec2 button_padding_px = vec2(10.f);
    UIText *text_obj;
    bool has_background = false;
    Panel *bg_panel = nullptr;

public:
    TextButton(const char* text_str, float font_scale, vec4 text_colour, int button_id, bool button_toggle, vec2 position=vec2(0.f))
        : Button(button_id, button_toggle, position, 0.f, Colour::TRANSPARENT) {
    
        text_obj = new UIText(text_str, font_scale, text_colour);
        text_obj->set_parent(this);
        text_obj->set_screenspace();

        resize_and_reposition();
    }
    
    void construct() override {
        Button::construct();
        text_obj->construct();
    }

    void set_shader(Shader *s) override {
        Button::set_shader(s);
        text_obj->set_shader(s);
        text_obj->initialize_shader_properties();
        if (has_background) bg_panel->set_shader(s);
    }
    
    void render() override {
        if (!visible) return;

        if (has_background) {
            bg_panel->calculate_transform_matrix();   
            bg_panel->enable_shader();
            bg_panel->update_transform();
            bg_panel->configure_render_properties();        
            bg_panel->render(); 
            bg_panel->disable_render_properties();
        }
        
        text_obj->calculate_transform_matrix();   
        text_obj->enable_shader();
        text_obj->update_transform();
        text_obj->configure_render_properties();        
        text_obj->render(); 
        text_obj->disable_render_properties();
        
        enable_shader();
        Button::render();
    }
    
    void set_text(std::string text_str) { 
        text_obj->set_text(text_str);
        resize_and_reposition();
    }

    void resize_and_reposition() override {
        
        set_size(text_obj->size + vec3(button_padding_px,0.f));

        text_obj->set_anchor(UIAnchor::CENTER, vec2(0));        

        if (has_background) {
            bg_panel->set_size(size);
            bg_panel->set_anchor(UIAnchor::CENTER, vec2(0.f));
        }
    }
    
    void set_clicked_state(bool clicked) override {
        is_pressed = clicked && toggle;
        if (has_background) bg_panel->set_tint_colour( clicked ? Colour::DARK_GREY : Colour::WHITE );
        else text_obj->set_tint_colour( clicked ? Colour::DARK_GREY : Colour::WHITE );
    }
    void set_hover_state(bool hover) override {
        if (is_pressed) return;
        if (has_background) bg_panel->set_tint_colour( hover ? Colour::BLACK : Colour::WHITE );
        else text_obj->set_tint_colour( hover ? Colour::BLACK : Colour::WHITE );
    }
    
    void set_background(vec4 colour) {
        if (!has_background){
            bg_panel = new Panel(colour, vec2(0), size);
            bg_panel->set_parent(this);
            bg_panel->set_screenspace();
            bg_panel->set_anchor(UIAnchor::CENTER, vec2(0.f));
            bg_panel->construct();
            if(this->shader) bg_panel->set_shader(this->shader);
            has_background = true;
        } 
        else bg_panel->set_colour(colour);
    }
};

#endif
