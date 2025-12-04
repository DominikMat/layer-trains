#ifndef UI_OBJECT_H
#define UI_OBJECT_H

#include "ScreenObject.h"
#include "ColourData.h"
#include <vector>
#include <algorithm>

enum class UIAnchor {
    TOP_LEFT,    TOP_CENTER,    TOP_RIGHT,
    MIDDLE_LEFT, CENTER,        MIDDLE_RIGHT,
    BOTTOM_LEFT, BOTTOM_CENTER, BOTTOM_RIGHT,
};

class UIObject : public ScreenObject {
public:
    UIAnchor anchor;
    glm::vec2 anchor_offset; // Offset from the anchor point in pixels

    bool uses_texture = false;
    Texture *texture = nullptr;
    glm::vec4 colour = Colour::WHITE;

    float scr_width = SCR_WIDTH;
    float scr_height = SCR_HEIGHT;

    UIObject(glm::vec2 size, UIAnchor anchor = UIAnchor::CENTER, glm::vec2 anchor_offset = glm::vec2(0.f))
        : ScreenObject(get_anchor_position(anchor)+anchor_offset, size), anchor(anchor), anchor_offset(anchor_offset)
    {}
    UIObject(glm::vec2 position, glm::vec2 size)
        : ScreenObject(position, size), anchor(UIAnchor::BOTTOM_LEFT), anchor_offset(vec2(0.f))
    {}

    void set_anchor (UIAnchor anchor, vec2 anchor_offset) {
        position = get_anchor_position(anchor) + anchor_offset;
        this->anchor = anchor;
        this->anchor_offset = anchor_offset;
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

    void update_screen_size(float w, float h) {
        scr_width = w; scr_height = h;
    }
};

#endif