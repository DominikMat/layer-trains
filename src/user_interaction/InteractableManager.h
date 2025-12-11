#ifndef InteractableManager_H
#define InteractableManager_H

#include "shaders/Shader.h" // The base class for all renderable entities
#include "rendering/Camera.h" // The base class for all renderable entities
#include "World.h" // The base class for all renderable entities
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
    const InteractionCallback callback_function;

    InteractableManager (World *world_ref, const InteractionCallback& callback_function) 
        : world_ref(world_ref), callback_function(callback_function) {}

    void process_all ( vec3 click_pos, bool call_objects_in_range ) {
        for (const auto& i : interactables) {
            float dx = click_pos.x - i->position.x;
            float dy = click_pos.y - i->position.y;
            float dz = click_pos.z - i->position.z;
            float distance_squared = dx*dx + dy*dy + dz*dz;
            i->process(distance_squared, call_objects_in_range);
        }
    }
    void resize_on_zoom( float current_zoom ){
        float resize_mult = glm::clamp(current_zoom, 0.3f, 1.8f);
        for (const auto& i: interactables) {
            i->set_size(resize_mult * INTERACTABLE_INTERACT_DISTANCE * INTERACTABLE_RENDER_RADUIS_MUTLIPLIER);
        }
    }
    Interactable* create(vec3 pos, const char* name, InteractionType interaction_type, float interact_dist) {
        Interactable* intr = new Interactable(pos, name, interaction_type, interact_dist, interactables.size());
        add(intr);
        return intr;
    }
    void add(Interactable* interactable) {
        this->interactables.push_back(interactable);
        interactable->add_callback (callback_function);
        interactable->set_id(interactables.size()-1);

        Object* obj = dynamic_cast<Object*>(interactable);
        if (!obj) {
            throw std::runtime_error("add: Interactable is not an Object");
        }
        obj->construct();
        world_ref->place(obj);
    }
    vector<Interactable*> get_current_interactables() { return interactables; }
};

#endif // InteractableManager_H
