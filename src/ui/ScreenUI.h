#ifndef ScreenUI_H
#define ScreenUI_H

#include "Shader.h"
#include "Object.h"
#include "ButtonPanel.h"
#include "Camera.h"
#include "InputHandler.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <stdexcept>

class Camera;

using namespace glm;
using ButtonCallback = std::function<void(int button_id, bool clicked)>;

class ScreenUI {
public:
    std::vector<Object*> objects;
    std::vector<Button*> buttons;
    Shader shader;
    Camera camera;
    ButtonCallback button_callback;

    ScreenUI () : shader(SCREEN_UI_SHADER), camera(Camera(SCR_WIDTH, SCR_HEIGHT, 0.f, 0.f, 0.f, 1.f)) { 
        camera.set_screenspace(&shader); // always ortho projection for 2D 
    }

    void render(float scr_width, float scr_height) {
        
        glDisable(GL_DEPTH_TEST);

        camera.set_screen_size(scr_width, scr_height); // update screen size in camera (and shader dependancy)
        camera.set_orthographic_zoom(scr_height); // cast to pixel coordinates 
        shader.use();

        for (const auto& object_ptr : objects) {
            object_ptr->calculate_transform_matrix();   
            object_ptr->enable_shader();
            object_ptr->update_transform();
            object_ptr->configure_render_properties();        
            object_ptr->render(); 
            object_ptr->disable_render_properties();
        }

        glEnable(GL_DEPTH_TEST);
    }

    void place(Object* obj) {
        this->objects.push_back(obj);
        obj->construct();
        obj->set_shader(&shader);
        obj->set_screenspace();
        obj->initialize_shader_properties();

        // detect and add buttons to seperate array
        Button* button = dynamic_cast<Button*>(obj);
        if (button) { buttons.push_back(button); return; }
        ButtonPanel* button_panel = dynamic_cast<ButtonPanel*>(obj);
        if (button_panel) { for (auto b : button_panel->get_buttons()) { buttons.push_back(b); } return; }
    }

    void check_button_clicked(InputHandler *ih) {
        vec2 mouse_pixel_pos = ih->get_mouse_position_pixels_inv_y();
        for (auto b : buttons) {
            if (b->is_mouse_over(mouse_pixel_pos.x, mouse_pixel_pos.y) && ih->is_left_mouse_clicked()) { // this way because is_mouse_over if true set the hover state automaticall

                b->set_clicked_state(!b->is_toggle() || !b->get_pressed_state());
                if (button_callback) button_callback(b->get_id(), !b->is_toggle() || b->get_pressed_state());
                break; // only one click possible at one time
            }
        }
    }

    void set_button_click_callback(ButtonCallback callback) {
        button_callback = callback;
    }
};

#endif // ScreenUI_H
