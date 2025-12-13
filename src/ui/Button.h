#ifndef BUTTON_H
#define BUTTON_H


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Object.h"
#include "UIObject.h"
#include "Texture.h"

using namespace glm;

class Button : public UIObject {
protected:
    unsigned int VAO, VBO, EBO;
    bool is_pressed = false, toggle = false;
    vec3 original_scale = vec3(1.0f);
    int id; 

public:
    Button(int button_id, bool toggle, vec2 pos, float size_px, Texture *texture)
        : UIObject(pos, vec2(size_px)), id(button_id), toggle(toggle) {
        this->set_texture(texture);
        //this->colour = vec4(1.0f); // Default white tint for texture
    }

    // Constructor for solid color button (no texture)
    Button(int button_id, bool toggle, vec2 pos, float size_px, vec4 colour)
        : UIObject(pos, vec2(size_px)), id(button_id), toggle(toggle) {
        this->set_colour(colour);
    }

    // --- Interaction Logic ---

    // Trigger visual feedback
    virtual void set_clicked_state(bool clicked) {
        is_pressed = clicked && toggle;
        set_tint_colour( clicked ? Colour::BLACK : Colour::WHITE );
    }
    virtual void set_hover_state(bool hover) {
        if (is_pressed) return;
        set_tint_colour( hover ? Colour::DARK_GREY : Colour::WHITE );
    }
    
    int get_id() override { return id; }
    bool get_pressed_state() { return is_pressed; }
    bool is_toggle() { return toggle; }

    /* click detection */
    // Simple AABB Collision Detection in Pixel Space
    bool is_mouse_over(double mouse_pixel_x, double mouse_pixel_y) {
        float left   = position.x - (size.x / 2.0f);
        float right  = position.x + (size.x / 2.0f);
        float top    = position.y + (size.y / 2.0f);
        float bottom = position.y - (size.y / 2.0f);
        
        bool mouse_over = (mouse_pixel_x >= left && mouse_pixel_x <= right && mouse_pixel_y >= bottom && mouse_pixel_y <= top);
        set_hover_state(mouse_over);
        return mouse_over;
    }

    // --- Standard Rendering (Quad) ---
    
    void construct() override {
        // Standard Quad geometry (Same as Panel)
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        float vertices[] = {
            // positions        // texture coords
            -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
             0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
             0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
            -0.5f,  0.5f, 0.0f,  0.0f, 1.0f
        };
        unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void render() override {
        if (!visible) return;
        
        // Optional: Apply dynamic click scale here if not handling in update
        // (Requires resetting shader matrix)
        
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    std::vector<Button*> get_buttons() override { 
        std::vector<Button*> return_btns = { this };
        return return_btns; 
    }

    ~Button() override {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
};

#endif