#ifndef ScreenUI_H
#define ScreenUI_H

#include "Shader.h"
#include "Object.h"
#include "Camera.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <stdexcept>

class Camera;

using namespace glm;

class ScreenUI {
public:
    std::vector<Object*> objects;
    Shader shader;
    Camera camera;

    ScreenUI () : shader(ShaderManager::get_screen_ui_shader()), camera(Camera(SCR_WIDTH, SCR_HEIGHT, 0.f, 0.f, 0.f, 1.f)) { 
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
    }
};

#endif // ScreenUI_H
