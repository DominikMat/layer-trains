#ifndef TextPanel_H
#define TextPanel_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

#include "Panel.h"
#include "UIText.h"

using namespace glm;

class TextPanel : public Panel
{
public:
    UIText *text_obj;
    vec2 padding;
    bool scale_to_text;

    TextPanel(std::string text_str, float font_scale, vec4 text_colour, vec4 panel_colour, vec2 size, bool scale_to_text = true, vec2 padding_percent = vec2(0.05f,0.5f), vec2 pos = vec2(0.f))
        : Panel(panel_colour, pos, size), padding(padding_percent), scale_to_text(scale_to_text) {
    
        text_obj = new UIText(text_str, font_scale, text_colour, false);
        text_obj->set_parent(this);
        text_obj->set_screenspace();

        resize_and_reposition();
    }
    
    void construct() override {
        Panel::construct();
        text_obj->construct();
    }

    void set_shader(Shader *s) override {
        Panel::set_shader(s);
        text_obj->set_shader(s);
        text_obj->initialize_shader_properties();
    }
    
    void render() override {
        if (!visible) return;

        Panel::render();

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
        float text_width = text_obj->get_text_width();
        if (text_width > size.x || scale_to_text) set_size(vec3(text_width*(1.f+padding.x),size.y,size.z));

        float text_height = text_obj->get_text_max_height();
        if (text_height > size.y || scale_to_text) set_size(vec3(size.x,text_height*(1.f+padding.y),size.z));

        recalculate_ui_position();
        text_obj->set_anchor( UIAnchor::BOTTOM_LEFT, vec2(size.x*padding.x/2.f, padding.y/2.f*size.y) );
    }
};

#endif
