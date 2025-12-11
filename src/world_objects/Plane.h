#ifndef PLANE_H
#define PLANE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include "world_objects/Object.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

class Plane : public Object
{
private:
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    unsigned int n;
    bool skip_render_boundary;

public:
    Plane(int vert_num_per_side = 2, vec3 pos = vec3(0.0f,0.0f,0.0f), vec3 size = vec3(1.0f,1.0f,1.0f), bool skip_render_boundary = false) 
        : Object(pos, size), n(glm::max(2,vert_num_per_side)), skip_render_boundary(skip_render_boundary)
    {}

    void construct() override {
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);
        glGenBuffers(1, &this->EBO);

        glBindVertexArray(this->VAO);

        subdivide();

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        // Do NOT unbind GL_ELEMENT_ARRAY_BUFFER while VAO is bound — it's stored in the VAO.
        glBindVertexArray(0); 
    }

    void render() override {
        if (!visible) return;
        glBindVertexArray(this->VAO); 
        int num_indices = (n-1)*(n-1)*6;
        glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    ~Plane() override {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

private:
    void subdivide() {        
        const float x_min = -.5f, w = 1.f;
        const float y_min = -.5f, h = 1.f;
        
        // Recompute vertices using floating-point division (avoid integer division bug above)
        std::vector<float> subdiv_vertices(n*n*5);
        subdiv_vertices.assign(n*n*5, 0.0f);

        int i = 0;
        for (int y = 0; y < n; ++y) {
            for (int x = 0; x < n; ++x) {
                float vert_pos_x = x_min+((float)x/(n-1))*w;
                float vert_pos_y = y_min+((float)y/(n-1))*h;

                subdiv_vertices[i++] = vert_pos_x;
                subdiv_vertices[i++] = vert_pos_y;
                subdiv_vertices[i++] = 0.f;
                subdiv_vertices[i++] = (vert_pos_x - x_min) / w;
                subdiv_vertices[i++] = (vert_pos_y - y_min) / h;
            }
        }
        
        // Fill indices: for each cell (x,y) produce two triangles: (bl, br, tr) and (tr, tl, bl)
        std::vector<unsigned int> subdiv_indices((n-1)*(n-1)*6);

        i = 0;
        for (int y = 0; y < n - 1; ++y) {
            for (int x = 0; x < n - 1; ++x) {
            unsigned int bl = y * n + x;        // bottom-left
            unsigned int br = bl + 1;           // bottom-right
            unsigned int tl = (y + 1) * n + x;  // top-left
            unsigned int tr = tl + 1;           // top-right

            subdiv_indices[i++] = bl;
            subdiv_indices[i++] = br;
            subdiv_indices[i++] = tr;

            subdiv_indices[i++] = tr;
            subdiv_indices[i++] = tl;
            subdiv_indices[i++] = bl;
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, subdiv_vertices.size() * sizeof(float), subdiv_vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, subdiv_indices.size() * sizeof(unsigned int), subdiv_indices.data(), GL_STATIC_DRAW);
    }
};

#endif
