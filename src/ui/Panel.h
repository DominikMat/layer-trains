#ifndef Panel_H
#define Panel_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include "UIObject.h"
#include "ColourData.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

class Panel : public UIObject
{
private:
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;


public:
    Panel(vec4 colour = Colour::WHITE, vec2 pos = vec2(SCR_WIDTH/2,SCR_HEIGHT/2), vec2 size = vec2(SCR_WIDTH/2,SCR_HEIGHT/2)) 
        : UIObject(pos, size){
        set_colour(colour);
    }

    void construct() override {
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);
        glGenBuffers(1, &this->EBO);

        glBindVertexArray(this->VAO);

            const float Panel_vertices[20] = {
            // positions         // tex coords
            -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // 0
            0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // 1
            0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // 2
            -0.5f,  0.5f, 0.0f,   0.0f, 1.0f  // 3
        };

        const unsigned int Panel_indicies[6] = {
            0, 1, 2,
            2, 3, 0
        };

        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(float), Panel_vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), Panel_indicies, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        glBindVertexArray(0); 
    }

    void render() override {
        if (!visible) return;
        glBindVertexArray(this->VAO); 
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    ~Panel() override {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
};

#endif
