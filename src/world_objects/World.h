#ifndef WORLD_H
#define WORLD_H

#include "world_objects/Object.h" // The base class for all renderable entities
#include "shaders/Shader.h" // The base class for all renderable entities
#include "rendering/Camera.h" // The base class for all renderable entities
#include <vector>
#include <memory>   // Required for std::unique_ptr
#include <glm/glm.hpp>
#include <stdexcept>

class Camera;

using namespace glm;

class World {
public:
    std::vector<Object*> objects;

    Camera *camera;

    World (Camera *camera) : camera(camera) { }
    
    void render(bool world_pos_pass = false) {
        int object_count_on_loop_start = objects.size();
        for (int i=0; i<object_count_on_loop_start; i++) {
            auto object_ptr = objects[i];
            
            if (world_pos_pass && !object_ptr->render_to_world_pos) {
                continue;
            }
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
    }
    
    void place(Object* obj) {
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
            obj->initialize_shader_properties();
            camera->set_orthographic(obj->shader);
        }

        // detect and place children of obj
        std::vector<Object*> obj_children = obj->get_children();
        for (auto c : obj_children) place(c);
        obj->new_child_added = false;
    }

    void clear_objects() {
        objects.clear();
    }
};

#endif // WORLD_H
