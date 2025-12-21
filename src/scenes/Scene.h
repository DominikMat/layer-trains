#ifndef SCENE_H
#define SCENE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "World.h"
#include "InputHandler.h"
#include "Camera.h"
#include "ScreenUI.h"

class Scene
{
protected:
    World *world;
    Camera *camera;
    ScreenUI *screen_ui;
    World *world_ui;
    InputHandler *user_input;

    bool is_active = true;
    vec4 background_colour = Colour::DEFAULT_RENDER_BACKGROUND;

    int next_scene_id = -1;
    
public:

    Scene (World *w, Camera *c, ScreenUI *s, InputHandler *ih) 
        : world(w), camera(c), screen_ui(s), user_input(ih) {}

    virtual void init() = 0;
    virtual void loop(float dt) = 0;
    
    bool active() { return is_active; }
    void end_scene(int next_scene_id = -1) { is_active = false; this->next_scene_id = next_scene_id; }
    int get_next_scene_id() { return next_scene_id; }

    virtual Shader* get_world_pos_buffer_shader() { return NULL; }
    
    virtual ~Scene() {}


    void set_background_colour(vec4 new_colour) {
        background_colour = new_colour;
    }
    vec4 get_background_colour() {
        return background_colour;
    }
};

#endif