#ifndef Interactable_H
#define Interactable_H

#include "shaders/Shader.h" 
#include "rendering/Camera.h" 
#include "Sphere.h" 
#include "ColourData.h" 
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <stdexcept>
#include <functional>

class Camera;

using namespace glm;

class Interactable : public Object {
public:

    using InteractionCallback = std::function<void()>;
    std::vector<InteractionCallback> callbacks;
    
    float interaction_distance = 0.f;
    float interaction_distance_sqr = 0.f;
    float highlight_distance_sqr = 0.f;

    Sphere render_sphere;

    Interactable(vec3 position, float interaction_distance = 1.f) : 
        Object(position,vec3(interaction_distance)), 
        interaction_distance(interaction_distance),
        render_sphere(position, interaction_distance*INTERACTABLE_RENDER_RADUIS_MUTLIPLIER) 
        {
        this->render_to_world_pos = false;
        interaction_distance_sqr = interaction_distance*interaction_distance;
        highlight_distance_sqr = interaction_distance_sqr;
    }

    void add_callback(const InteractionCallback& callback_function) {
        callbacks.push_back(callback_function);
    }

    void call() {
        render_sphere.set_colour( Colour::RED );
        for (const auto& callback : callbacks) { 
            if (callback) callback();
        }
    }

    void process (float distance_sqr, bool call_in_range ) {
        //highlight obj if in range
        if (distance_sqr <= highlight_distance_sqr){
            render_sphere.set_colour( INTERACTABLE_HIGHLIGHTED_COLOUR );
            render_sphere.set_transparency ( INTERACTABLE_HIGHLIGHTED_OBJECT_ALPHA );
        } else {
            render_sphere.set_colour( INTERACTABLE_DEFUALT_COLOUR );
            render_sphere.set_transparency ( INTERACTABLE_OBJECT_ALPHA );
        }

        // call if in range and flag set
        if (call_in_range && distance_sqr <= interaction_distance_sqr) call();
    }

    void configure_render_properties () override {
        render_sphere.configure_render_properties();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
    }
    void disable_render_properties () override {
        render_sphere.disable_render_properties();
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }
    void render () override {
        render_sphere.set_position(position);
        render_sphere.set_size(size);
        render_sphere.render();
    }
    void construct () override {
        render_sphere.construct();
    }
};

#endif // Interactable_H