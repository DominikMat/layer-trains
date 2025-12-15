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
    bool scale_to_text, center_text;
    int padding_left_px;

    TextPanel(std::string text_str, float font_scale, vec4 text_colour, vec4 panel_colour, vec2 size, bool scale_to_text = true, 
        bool center = false, float padding_left_px = 10, vec2 padding_px = vec2(10), vec2 pos = vec2(0.f))
        :   Panel(panel_colour, pos, size), padding(padding_px), padding_left_px(padding_left_px), 
            scale_to_text(scale_to_text), center_text(center)         
    {
        text_obj = new UIText(text_str, font_scale, text_colour);
        text_obj->set_ui_parent(this);
        text_obj->set_screenspace();

        resize_and_reposition();
    }
    
    void set_text(std::string text_str) { 
        text_obj->set_text(text_str);
        resize_and_reposition();
    }

    void resize_and_reposition() override {
        if (text_obj->size.x > size.x || scale_to_text) set_size(vec3(text_obj->size.x+(2.f*padding.x),size.y,size.z));
        if (text_obj->size.y > size.y || scale_to_text) set_size(vec3(size.x,text_obj->size.y+(2.f*padding.y),size.z));

        recalculate_ui_position();
        if (center_text) text_obj->set_anchor( UIAnchor::CENTER, vec2(0.f) );
        else text_obj->set_anchor( UIAnchor::MIDDLE_LEFT, vec2(padding_left_px, 0.f) );
    }
};

#endif
