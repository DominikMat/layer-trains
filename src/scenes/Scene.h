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
public:
    World *world;
    Camera *camera;
    ScreenUI *screen_ui;
    InputHandler *user_input;

    bool is_active = true;

    Scene (World *w, Camera *c, ScreenUI *s, InputHandler *ih) 
        : world(w), camera(c), screen_ui(s), user_input(ih) {}

    virtual void init() = 0;
    virtual void loop(float dt) = 0;
    
    bool active() { return is_active; }
    void exit() { on_exit(); is_active = false; }
    void on_exit() {}

    virtual Shader* get_world_pos_buffer_shader() { return NULL; }
    
    virtual ~Scene() {}
};

#endif