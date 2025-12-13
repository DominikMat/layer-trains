#ifndef INPUT_H
#define INPUT_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <algorithm>
#include "settings/Utility.h"
#include "settings/Settings.h"
#include "Window.h"

const float DOUBLE_CLICK_WINDOW = 0.3f;
using namespace glm;

enum DoubleClickState {
    IDLE=0, FIRST_CLICK_PRESSED=1, FIRST_CLICK_RELEASED=2, SECOND_CLICK_PRESSED=3, SECOND_CLICK_RELEASED=4
};

struct MouseClick {
    bool is_clicked = false;
    bool is_double_clicked = false;
    bool is_held = false;

    DoubleClickState double_click_state = DoubleClickState::IDLE;
    float double_click_timer = 0.f;
};

class InputHandler {
public:
    bool simulation_paused = false, was_space_pressed = false, holding_shift = false;
    float scroll_value = 1.f, scroll_speed_multiplier = 10.f;
    vec2 mouse_position_normalized = vec2(0.f), last_mouse_position_pixels = vec2(0.f), mouse_position_pixels = vec2(0.f), mouse_position_pixels_inv_y = vec2(0.f);     // Surowe piksele ekranu
    
    MouseClick mouse_left;
    MouseClick mouse_right;
    MouseClick mouse_middle;

    static InputHandler *instance;

    InputHandler() { instance = this; }

    void process_input(GLFWwindow *glfw_window, float dt, vec2 window_size){

        // shift
        holding_shift = glfwGetKey(glfw_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

        // pause 
        if(glfwGetKey(glfw_window, GLFW_KEY_SPACE) == GLFW_PRESS) simulation_paused = !simulation_paused;

        // get mouse position
        double xpos, ypos;
        glfwGetCursorPos(glfw_window, &xpos, &ypos);
        last_mouse_position_pixels = mouse_position_pixels;
        mouse_position_normalized = vec2((float)(xpos / SCR_WIDTH)*2.f, (0.5f - (float)(ypos / SCR_HEIGHT))*2.f);
        mouse_position_normalized += vec2(0.011f, -0.015f); // offset idk why ???
        mouse_position_normalized.x = glm::clamp(mouse_position_normalized.x, 0.f, 1.f);
        mouse_position_normalized.y = glm::clamp(mouse_position_normalized.y, 0.f, 1.f);
        mouse_position_pixels = vec2(glm::clamp((float)xpos,0.f,window_size.x), glm::clamp((float)ypos,0.f,window_size.y));
        mouse_position_pixels_inv_y = vec2(glm::clamp((float)xpos,0.f,window_size.x), glm::clamp(window_size.y - (float)ypos,0.f,window_size.y));

        // mouse button logic
        update_mouse_click_logic(&mouse_left, glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_LEFT), dt);
        update_mouse_click_logic(&mouse_middle, glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_MIDDLE), dt);
        update_mouse_click_logic(&mouse_right, glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_RIGHT), dt);
    }
    
    // Scroll wheel logic
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        if (instance) instance->update_scroll_value((float)yoffset);
    }
    void update_scroll_value(float yoffset) {
        scroll_value -= yoffset * scroll_speed_multiplier; 
        //scroll_value = std::clamp(scroll_value, 0.f, 2.f);
    }
    void reset_scroll_value() { scroll_value = 1.f;}
    void set_scroll_speed(float new_speed) { scroll_speed_multiplier = new_speed; }
    float get_scroll_speed() { return scroll_speed_multiplier; }

    // Znormalizowana pozycja myszy (do shadera)
    vec2 get_mouse_position_normalized() { return mouse_position_normalized; }
    vec2 get_mouse_position_pixels() { return mouse_position_pixels; }
    vec2 get_mouse_position_pixels_inv_y() { return mouse_position_pixels_inv_y; }
    vec2 get_mouse_movement_since_last_frame() { return (mouse_position_pixels-last_mouse_position_pixels); }

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

    // mouse state getters
    bool is_left_mouse_clicked() { return mouse_left.is_clicked; }
    bool is_left_mouse_held() { return mouse_left.is_held; }
    bool is_left_mouse_double_clicked() { return mouse_left.is_double_clicked; }

    bool is_middle_mouse_clicked() { return mouse_middle.is_clicked; }
    bool is_middle_mouse_held() { return mouse_middle.is_held; }
    bool is_middle_mouse_double_clicked() { return mouse_middle.is_double_clicked; }

    bool is_right_mouse_clicked() { return mouse_right.is_clicked; }
    bool is_right_mouse_held() { return mouse_right.is_held; }
    bool is_right_mouse_double_clicked() { return mouse_right.is_double_clicked; }

    // other input getters
    bool get_simulation_paused() { return simulation_paused; }
    float get_scroll_value() { return scroll_value; }
    bool is_holding_shift() { return holding_shift; }

private:
    void update_mouse_click_logic(MouseClick *state, int curr_val, float dt) {
        
        bool was_held = state->is_held;
        state->is_held = curr_val == GLFW_PRESS;
        
        bool just_pressed = !was_held && state->is_held;
        bool just_released = was_held && !state->is_held;

        state->is_clicked = just_released; // is_clicked true on key released (one frame)
        
        /* DOUBLE CLICK LOGIC */
        
        // reset double click logic after one frame (if was true on the last one)
        if (state->is_double_clicked) state->double_click_state = DoubleClickState::IDLE;

        // start tracking state on click
        if (just_pressed && state->double_click_state == DoubleClickState::IDLE){
            state->double_click_state = DoubleClickState::FIRST_CLICK_PRESSED;
            state->double_click_timer = 0.f;
        }

        // double click state update logic
        state->double_click_timer += dt;
        if (state->double_click_timer > DOUBLE_CLICK_WINDOW) {
            state->double_click_state = DoubleClickState::IDLE; // reset state if timer expired
        }
        else { 
            // if relased -> update double click state to released (so +1 because the enums form a sequence)
            if (just_released && (state->double_click_state == DoubleClickState::FIRST_CLICK_PRESSED || state->double_click_state == DoubleClickState::SECOND_CLICK_PRESSED)) {
                state->double_click_state = (DoubleClickState)((int)(state->double_click_state) + 1);
            }

            // check if second click pressed
            if (just_pressed && state->double_click_state == DoubleClickState::FIRST_CLICK_RELEASED)
                state->double_click_state = DoubleClickState::SECOND_CLICK_PRESSED;
        }

        state->is_double_clicked = state->double_click_state == DoubleClickState::SECOND_CLICK_RELEASED;
    }

};

#endif // WORLD_H

        // if(glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        //     glfwSetWindowShouldClose(w, true);
        
        // // camera movement
        // camera_mv = vec3(0.f);
        // if(glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS)
        //     camera_mv += V3_Z;
        // if(glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS)
        //     camera_mv -= V3_Z;
        // if(glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS)
        //     camera_mv += V3_X;
        // if(glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS)
        //     camera_mv -= V3_X;

        // if(glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        //     camera_mv += V3_Y;
        // if(glfwGetKey(w, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        //     camera_mv -= V3_Y;

