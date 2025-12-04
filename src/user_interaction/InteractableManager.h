#ifndef InteractableManager_H
#define InteractableManager_H

#include "shaders/Shader.h" // The base class for all renderable entities
#include "rendering/Camera.h" // The base class for all renderable entities
#include "Interactable.h" // The base class for all renderable entities
#include <vector>
#include <memory>   // Required for std::unique_ptr
#include <glm/glm.hpp>
#include <stdexcept>

class Object;

using namespace glm;

class InteractableManager {
public:
    std::vector<Interactable*> interactables;
    World *world_ref;

    InteractableManager (World *world_ref) { this->world_ref = world_ref; }

    void process_all ( vec3 click_pos, bool call_objects_in_range ) {
        for (const auto& i : interactables) {
            float dx = click_pos.x - i->position.x;
            float dy = click_pos.y - i->position.y;
            float dz = click_pos.z - i->position.z;
            float distance_squared = dx*dx + dy*dy + dz*dz;
            i->process(distance_squared, call_objects_in_range);
        }
    }
    Interactable* create(vec3 pos, float interact_dist) {
        Interactable* intr = new Interactable(pos, interact_dist);
        add(intr);
        return intr;
    }
    void add(Interactable* interactable) {
        this->interactables.push_back(interactable);

        Object* obj = dynamic_cast<Object*>(interactable);
        if (!obj) {
            throw std::runtime_error("add: Interactable is not an Object");
        }
        obj->construct();
        world_ref->place(obj);
    }
};

#endif // InteractableManager_H
