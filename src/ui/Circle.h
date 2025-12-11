#ifndef CIRCLE_H
#define CIRCLE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Object.h"
#include "UIObject.h"
#include <vector>
#include <cmath>

using namespace glm;

class Circle : public UIObject {
private:
    unsigned int VAO, VBO;
    int segments;

public:
    // Size here represents the diameter (width and height)
    Circle(vec2 pos, float diameter, vec4 color = Colour::WHITE, int segments = 36)
        : UIObject(pos, vec2(diameter)), segments(segments) {
        set_colour(color);
    }

    void construct() override {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);

        std::vector<float> vertices;
        
        // Center vertex (0,0) with UV (0.5, 0.5)
        vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(0.0f); // Pos
        vertices.push_back(0.5f); vertices.push_back(0.5f); // UV

        float radius = 0.5f; // Normalized size, scaled by transform matrix later
        float angleStep = 360.0f / segments;

        for (int i = 0; i <= segments; i++) {
            float angle = glm::radians(i * angleStep);
            float x = cos(angle) * radius;
            float y = sin(angle) * radius;
            
            // Map x,y (-0.5 to 0.5) to UV (0.0 to 1.0)
            float u = x + 0.5f;
            float v = y + 0.5f;

            vertices.push_back(x); vertices.push_back(y); vertices.push_back(0.0f);
            vertices.push_back(u); vertices.push_back(v);
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // Texture attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void render() override {
        if (!visible) return;
        glBindVertexArray(VAO);
        // Render Triangle Fan (Center + perimeter vertices)
        glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);
        glBindVertexArray(0);
    }

    ~Circle() override {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
};

#endif