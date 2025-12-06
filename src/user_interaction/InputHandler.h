#ifndef INPUT_H
#define INPUT_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "settings/Utility.h"
#include "settings/Settings.h"
#include <algorithm> // Dla std::clamp

using namespace glm;

class InputHandler {
private:
    vec3 camera_mv;
    bool simulation_paused, was_space_pressed;
    bool holding_shift;
    float scroll_value, scroll_speed_multiplier;
    vec2 mouse_position_normalized; // Twoja oryginalna dziwna normalizacja (0-1)
    vec2 mouse_position_pixels;     // Surowe piksele ekranu
    bool was_mouse_pressed, is_mouse_pressed;         // Do wykrywania krawędzi kliknięcia

    static InputHandler *instance;

public:

    InputHandler() : 
        camera_mv(vec3(0.f)), 
        simulation_paused(false), was_space_pressed(false), 
        scroll_value(1.0f), scroll_speed_multiplier(0.1f),
        was_mouse_pressed(false), is_mouse_pressed(false)
    {
        instance = this;
    }

    void process_input(GLFWwindow *w){
        if(glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(w, true);
        
        // camera movement
        camera_mv = vec3(0.f);
        if(glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS)
            camera_mv += V3_Z;
        if(glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS)
            camera_mv -= V3_Z;
        if(glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS)
            camera_mv += V3_X;
        if(glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS)
            camera_mv -= V3_X;

        // if(glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        //     camera_mv += V3_Y;
        // if(glfwGetKey(w, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        //     camera_mv -= V3_Y;

        // pause
        if(glfwGetKey(w, GLFW_KEY_SPACE) == GLFW_PRESS){
            if (!was_space_pressed) simulation_paused = !simulation_paused; 
            was_space_pressed = true;
        }
        else was_space_pressed = is_mouse_pressed = false;
        
        holding_shift = glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

        // get mouse position
        double xpos, ypos;
        glfwGetCursorPos(w, &xpos, &ypos);
        mouse_position_normalized = vec2((float)(xpos / SCR_WIDTH)*2.f, (0.5f - (float)(ypos / SCR_HEIGHT))*2.f);
        mouse_position_normalized += vec2(0.011f, -0.015f); // offset idk why ???
        mouse_position_normalized.x = glm::clamp(mouse_position_normalized.x, 0.f, 1.f);
        mouse_position_normalized.y = glm::clamp(mouse_position_normalized.y, 0.f, 1.f);
        mouse_position_pixels = vec2(xpos, ypos);

        // Wykrywanie kliknięcia - krawędź
        if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            is_mouse_pressed = !was_mouse_pressed;
            was_mouse_pressed = true;
        } else {
            was_mouse_pressed = false;
        }

    }

    // Statyczna funkcja zwrotna (callback) GLFW dla kółka myszy
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        if (instance) instance->update_scroll_value((float)yoffset);
    }

    // Metoda do aktualizacji wartości scroll_value (zoom)
    void update_scroll_value(float yoffset) {
        scroll_value -= yoffset * scroll_speed_multiplier; 
        scroll_value = std::clamp(scroll_value, 0.f, 2.f);
    }
    void reset_scroll_value() {
        scroll_value = 1.f;
    }
    void set_scroll_speed(float new_speed) {
        scroll_speed_multiplier = new_speed;
    }
    float get_scroll_speed() {
        return scroll_speed_multiplier;
    }

    vec3 get_camera_movement() { return camera_mv; }
    bool get_simulation_paused() { return simulation_paused; }
    float get_scroll_value() { return scroll_value; }
    bool is_holding_shift() { return holding_shift; }
    bool is_left_mouse_clicked() { return is_mouse_pressed; }

    // Znormalizowana pozycja myszy (do shadera)
    vec2 get_mouse_position_normalized() { return mouse_position_normalized; }
    vec2 get_mouse_position_pixels() { return mouse_position_pixels; }

    vec3 get_mouse_position_world() {
        // 2. Odczytaj piksel pod myszą
        int mouse_y_gl = SCR_HEIGHT - (int)mouse_position_pixels.y; // Odwróć oś Y GL
        
        float pixelData[3];
        glReadBuffer(GL_COLOR_ATTACHMENT15);

        // Sprawdzenie granic, aby uniknąć błędów glReadPixels
        if (mouse_position_pixels.x < 0 || mouse_position_pixels.x >= SCR_WIDTH || mouse_y_gl < 0 || mouse_y_gl >= SCR_HEIGHT) {
            return vec3(-1.0f);
        }
        glReadPixels((int)mouse_position_pixels.x, mouse_y_gl, 1, 1, GL_RGB, GL_FLOAT, &pixelData);
        return vec3(pixelData[0], pixelData[1], pixelData[2]);
    }

};

#endif // WORLD_H
