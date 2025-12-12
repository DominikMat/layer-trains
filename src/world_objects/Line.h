#ifndef LINE_H
#define LINE_H

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

class Line : public Object
{
private:
    unsigned int vao = 0;
    unsigned int vbo = 0;
    std::vector<vec3> points;
    float line_thickness = 5.f;

public:
    Line(std::vector<vec3> points, float line_thickness = 3.f)
        : Object(vec3(0.f), vec3(1.f)), line_thickness(line_thickness), points(points)
    { render_to_world_pos = false; points.clear(); }
    
    Line(float line_thickness = 3.f)
        : Object(vec3(0.f), vec3(1.f)), line_thickness(line_thickness), points(NULL)
    { render_to_world_pos = false; points.clear(); }

    void construct() override {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    void render() override {
        if (points.size() < 2) return;
       
        //glDisable(GL_DEPTH_TEST);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_DYNAMIC_DRAW);

        glLineWidth(line_thickness);
        
        glDrawArrays(GL_LINE_STRIP, 0, points.size());
       
        glBindVertexArray(0);
        //glEnable(GL_DEPTH_TEST);
    }

    void add_point(vec3 p) { points.push_back(p); }
    void add_points(std::vector<vec3> new_points) { for (auto p : new_points) points.push_back(p); }
    void set_points(std::vector<vec3> new_points) { this->points = new_points; }
    void clear_points() { points.clear(); }
    int get_point_num() { return points.size(); }
    std::vector<vec3> get_points() { return points; }
    vec3 get_last_point() { return points.back(); }

    ~Line() override {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }
};

#endif 