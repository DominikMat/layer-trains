#ifndef TextButton_H
#define TextButton_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

#include "Button.h"
#include "UIText.h"

using namespace glm;

class TextButton : public Button
{
public:
    const vec2 button_relative_size = vec2(1.1f);
    UIText *text_obj;

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
    }
    
    void render() override {
        if (!visible) return;

        Button::render();

        text_obj->calculate_transform_matrix();   
        text_obj->enable_shader();
        text_obj->update_transform();
        text_obj->configure_render_properties();        
        text_obj->render(); 
        text_obj->disable_render_properties();

        enable_shader();
    }
    
    void set_text(std::string text_str) { 
        text_obj->set_text(text_str);
        resize_and_reposition();
    }

    void resize_and_reposition() override {
        
        set_size(text_obj->size * vec3(button_relative_size,1.f));
        recalculate_ui_position();

        text_obj->set_anchor(UIAnchor::CENTER, vec2(0));        
    }

    void set_clicked_state(bool clicked) override {
        is_pressed = clicked && toggle;
        text_obj->set_tint_colour( clicked ? Colour::DARK_GREY : Colour::WHITE );
    }
    void set_hover_state(bool hover) override {
        if (is_pressed) return;
        text_obj->set_tint_colour( hover ? Colour::BLACK : Colour::WHITE );
    }
};

#endif
