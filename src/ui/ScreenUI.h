#ifndef ScreenUI_H
#define ScreenUI_H

#include "Camera.h"
#include "UIObject.h"
#include "Button.h"
#include "InputHandler.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <stdexcept>

using namespace glm;
using ButtonCallback = std::function<void(int button_id, bool clicked)>;

class ScreenUI {
public:
    std::vector<UIObject*> objects;
    std::vector<Button*> buttons;
    Camera *camera;
    ButtonCallback button_callback;
    float scr_width, scr_height;

    ScreenUI (float window_x, float window_y) : camera(new Camera(window_x, window_y, 0.f, 0.f, 0.f, 1.f)), 
        scr_width(scr_width), scr_height(scr_height) {}

    void render() {
        
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        int object_count_on_loop_start = objects.size();
        for (int i=0; i<object_count_on_loop_start; i++) {
            auto object_ptr = objects[i];
            
            if (object_ptr->new_child_added) {
                place(object_ptr); // will recursively place all children, skip already added
            }
            
            object_ptr->calculate_transform_matrix();   
            object_ptr->enable_shader();
            object_ptr->update_transform();
            object_ptr->configure_render_properties();        
            object_ptr->render(); 
            object_ptr->disable_render_properties();
        }
        
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }

    void place(UIObject* obj) {
        if (!obj) {
            std::cout << "ATTEMPTED TO ADD NULL OBJECT TO SCREEN UI!" << std::endl;
            return;
        }
        bool exists = false;
        for (auto o : objects) {
            if (o == obj) { 
                exists = true; 
                break; 
            }
        }
        
        if (!exists) {
            this->objects.push_back(obj);
            obj->construct();
            Shader *new_ui_shader = new SCREEN_UI_SHADER;
            camera->set_screenspace(new_ui_shader);
            obj->set_shader(new_ui_shader);
            obj->set_screenspace();
            obj->initialize_shader_properties();
        }

        // detect and add buttons to seperate array
        Button* obj_button = obj->get_button();
        if(obj_button) buttons.push_back(obj_button);

        // detect and place buttons to seperate array
        vector<UIObject*> obj_children = obj->get_ui_children();
        for (auto c : obj_children) place(c);
        obj->new_child_added = false;
    }

    void check_button_clicked(InputHandler *ih) {
        vec2 mouse_pixel_pos = ih->get_mouse_position_pixels_inv_y();
        for (auto b : buttons) {
            if (b->is_mouse_over(mouse_pixel_pos) && ih->is_left_mouse_pressed_up()) { // this way because is_mouse_over if true set the hover state automaticall

                b->set_clicked_state(!b->is_toggle() || !b->get_pressed_state());
                if (button_callback) button_callback(b->get_id(), !b->is_toggle() || b->get_pressed_state());
                break; // only one click possible at one time
            }
        }
    }

    void set_button_click_callback(ButtonCallback callback) {
        button_callback = callback;
    }

    void clear_objects() {
        objects.clear();
    }

    void set_new_window_size(float x, float y) {
        if (x<0 || y<0) return;

        scr_width = x;
        scr_height = y;

        camera->set_screen_size(scr_width, scr_height); // update screen size in camera (and shader dependancy)
        camera->set_orthographic_zoom(scr_height); // cast to pixel coordinates 

        for (auto object_ptr : objects) {
            object_ptr->update_screen_size(scr_width, scr_height);
        }
    }

    void update_animations(float dt) {
        for (auto obj : objects) obj->update_animations(dt);
    }
};

#endif // ScreenUI_H
