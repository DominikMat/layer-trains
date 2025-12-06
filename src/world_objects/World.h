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
        for (const auto& object_ptr : objects) {
            if (world_pos_pass && !object_ptr->render_to_world_pos) {
                continue;
            }
         
            object_ptr->calculate_transform_matrix();   
            object_ptr->enable_shader();
            object_ptr->configure_render_properties();
            object_ptr->update_transform();
            object_ptr->render(); 
            object_ptr->disable_render_properties();
        }
    }
    
    void place(Object* obj, int render_priority = 0) {
        this->objects.push_back(obj);
        obj->construct();
        camera->set_orthographic(obj->shader);

        // std::sort(objects.begin(), objects.end(), 
        //     [](const Object* a, const Object* b) { 
        //     return a->render_priority < b->render_priority; 
        //     });
    }
};

#endif // WORLD_H
