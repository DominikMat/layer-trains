

#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>
#include "world_objects/Cube.h"
#include "world_objects/Object.h"
#include "world_objects/Plane.h"
#include "world_objects/World.h"
#include "shaders/Shader.h"
#include "shaders/ShaderManager.h"
#include "settings/Settings.h"
#include "settings/Utility.h"
#include "rendering/Camera.h"
#include "user_interaction/InputHandler.h"
#include "textures/Texture.h"
#include "Terrain.h"
#include "world_objects/Line.h"
#include "Panel.h"
#include "Sphere.h"
#include "Interactable.h"
#include "InteractableManager.h"
#include "Text.h"
#include "ScreenUI.h"
#include "settings/ColourData.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Window
{
public:
    unsigned int scr_width;
    unsigned int scr_height;
    std::string window_title;
    GLFWwindow* window;

    Window(int scr_width, int scr_height, const char* window_title)
        : scr_width(scr_width), scr_height(scr_height), window_title(window_title)
    {
    }

    void initGLFW() {
        glfwInit();

        // select OpenGL version 3.3
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

        // select OpenGL Core Profile
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // enable backface culling
        //glEnable(GL_DEPTH_TEST)
        //glEnable(GL_CULL_FACE);
        //glFrontFace(GL_CCW);
    }

    bool create() {
        this->window = glfwCreateWindow(this->scr_width, this->scr_height, this->window_title.c_str(), NULL, NULL);

        if (this->window == NULL) {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return false;
        }

        glfwSetFramebufferSizeCallback(window, Window::framebuffer_size_callback);

        glEnable(GL_DEPTH_TEST);

        return true;
    }

    GLFWwindow* get() {
        return this->window;
    }

    void display_start() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void display(){
        glfwSwapBuffers(this->window);
        glfwPollEvents();
    }

    void clear(float r=0.0f, float b=0.0f, float g=0.0f, float a=1.0f) {
        glClearColor(r,g,b,a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    bool open() {
        return !glfwWindowShouldClose(this->window);
    }

private:
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height){
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
    }
};
#endif