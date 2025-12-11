#ifndef BUTTON_PANEL_H
#define BUTTON_PANEL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "UIObject.h"
#include "Button.h"
#include "Circle.h"
#include "Object.h"
#include "Panel.h"
#include <vector>

class ButtonPanel : public UIObject {
private:
    // Layout settings
    float gap_size;
    float button_size;
    float panel_height;
    vec4 bg_color;
    vec4 btn_color;
    
    // Components
    Panel* backgroundRect;
    Circle* leftCap;
    Circle* rightCap;
    std::vector<Button*> buttons;

public:
    ButtonPanel(vec2 pos, float button_size, float gap, vec4 background_col, vec4 button_col)
        : UIObject(pos, vec2(0, button_size + 2 * gap)), 
          button_size(button_size), gap_size(gap), 
          bg_color(background_col), btn_color(button_col) 
    {
        // 1. Create Background Rectangle
        panel_height = button_size * 1.2f;
        backgroundRect = new Panel(bg_color, vec2(0,0), vec2(0, panel_height));
        backgroundRect->set_parent(this); 
        backgroundRect->set_anchor(UIAnchor::CENTER, vec2(0.f));

        // 2. Create End Caps
        leftCap = new Circle(vec2(0,0), panel_height, bg_color);
        leftCap->set_parent(this);
        leftCap->set_anchor(UIAnchor::MIDDLE_LEFT, vec2(0.f));
        
        rightCap = new Circle(vec2(0,0), panel_height, bg_color);
        rightCap->set_parent(this);
        rightCap->set_anchor(UIAnchor::MIDDLE_RIGHT, vec2(0.f));
    }

    void add_button(int id, bool toggle, Texture *icon) {
        Button* btn = new Button(id, toggle, vec2(0), button_size, icon);
        setup_new_button(btn);
    }
    
    void add_button(int id, bool toggle, vec4 specific_color) {
        Button* btn = new Button(id, toggle, vec2(0), button_size, specific_color);
        setup_new_button(btn);
    }

    // Helper to position and store button
    void setup_new_button(Button* btn) {
        btn->set_parent(this);
        btn->set_screenspace(); 
        buttons.push_back(btn);
        
        recalculate_layout();
        
        btn->construct(); 
        btn->set_shader(this->shader);
    }

    void recalculate_layout() {
        int n = buttons.size();
        
        /* calculate and set new width */
        float inner_width = n==0 ? 0.f : n*button_size + (n-1)*(gap_size);
        float full_width = inner_width + panel_height; 
        set_size(vec3(full_width, panel_height, 1.f));
        backgroundRect->set_size(vec3(inner_width, panel_height, 1.f));
        this->recalculate_ui_position(); // make sure we stay centered

        /* calc button positions */
        for (int i=0; i<n; i++) {
            float offset_x = i*(button_size+gap_size)+(button_size+gap_size)/2.f;
            buttons[i]->set_anchor(UIAnchor::MIDDLE_LEFT, vec2(offset_x,0.f));
        } 

        /* update child positions */
        leftCap->recalculate_ui_position();
        rightCap->recalculate_ui_position();
        backgroundRect->recalculate_ui_position();
    }

    // --- Override Core Object Functions ---
    void construct() override {
        backgroundRect->construct();
        leftCap->construct();
        rightCap->construct();
    }

    void set_shader(Shader* s) override {
        Object::set_shader(s);
        backgroundRect->set_shader(s);
        leftCap->set_shader(s);
        rightCap->set_shader(s);
        for(auto b : buttons) b->set_shader(s);
    }

    void calculate_transform_matrix() override {
        Object::calculate_transform_matrix(); 
        backgroundRect->calculate_transform_matrix();
        leftCap->calculate_transform_matrix();
        rightCap->calculate_transform_matrix();
        for (auto btn : buttons) btn->calculate_transform_matrix();
    }

    void render() override {
        if (!visible) return;

        // 1. Background
        backgroundRect->update_transform(); // Uploads BG matrix
        backgroundRect->configure_render_properties(); // Uploads BG colour
        backgroundRect->render();

        // 2. Caps
        leftCap->update_transform();
        leftCap->configure_render_properties();
        leftCap->render();

        rightCap->update_transform();
        rightCap->configure_render_properties();
        rightCap->render();

        // 3. Buttons
        for (auto btn : buttons) {
            btn->update_transform();
            btn->configure_render_properties();
            btn->render();
        }
    } 

    const std::vector<Button*>& get_buttons() { return buttons; }

    int get_mouse_over_button(int mouse_pixel_x, int mouse_pixel_y) {
        for (auto b : buttons) {
            if (b->is_mouse_over(mouse_pixel_x, mouse_pixel_y)) return b->get_id();
        }
        return -1;
    }

    ~ButtonPanel() override {
        delete backgroundRect;
        delete leftCap;
        delete rightCap;
        for (auto b : buttons) delete b;
    }
};

#endif