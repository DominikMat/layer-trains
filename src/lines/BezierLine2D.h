#ifndef BezierLine2D_H
#define BezierLine2D_H

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

class BezierLine2D : public Object
{
private:
    unsigned int vao = 0;
    unsigned int vbo = 0;
    std::vector<vec2> points;
    std::vector<vec2> control_points;
    std::vector<vec2> render_points;
    float line_thickness = 5.f;
    bool points_changed = false;

public:
    BezierLine2D(vec2 point1, vec2 control, vec2 point2, float line_thickness = 3.f, vec3 line_colour = Colour::WHITE)
        : Object(vec3(0.f), vec3(1.f)), line_thickness(line_thickness), points(points)
    { 
        render_to_world_pos = false;
        set_segment(point1, control, point2);
        set_colour(line_colour);
    }
    
    BezierLine2D(float line_thickness = 3.f)
        : Object(vec3(0.f), vec3(1.f)), line_thickness(line_thickness), points(NULL)
    { render_to_world_pos = false; points.clear(); }

    void construct() override {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    void render() override {
        if (!visible) return;
        if (points_changed) regenerate_bezier();

        if (render_points.size() < 2) return;
       
       
        //glDisable(GL_DEPTH_TEST);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, render_points.size() * sizeof(glm::vec2), render_points.data(), GL_STATIC_DRAW);

        glLineWidth(line_thickness);
        
        glDrawArrays(GL_LINE_STRIP, 0, render_points.size());
       
        glBindVertexArray(0);
        //glEnable(GL_DEPTH_TEST);
    }

    void regenerate_bezier() {
        if (points.size() < 2 || control_points.size() < 1) {
            render_points.clear();
            return;
        }

        const int render_points_per_segment = 20;

        render_points.clear();
        render_points.resize(points.size() + (points.size()-1)*render_points_per_segment);
        int point_idx = 0;
        
        // for each segment of 2 points and control points
        for (int i=1; i<points.size(); i++) {
            vec2 start_point = points[i-1];
            vec2 end_point = points[i];

            render_points[point_idx++] = start_point;
            
            if (i-1 < control_points.size()-1) break;
            vec2 control_point = control_points[i-1];
            
            // go through bezier lerp logic and push render points
            for (int j=1; j<=render_points_per_segment; j++) {
                float t = (float)j / (render_points_per_segment+1);
                float invt = 1.f - t;
                vec2 lerp_beg = start_point*invt + control_point*t;
                vec2 lerp_end = control_point*invt + end_point*t;
                vec2 lerp_final = lerp_beg*invt + lerp_end*t;
                render_points[point_idx++] = lerp_final;
            }

            render_points[point_idx++] = end_point;
        }
        
    }

    void set_segment(vec2 p1, vec2 c, vec2 p2) {
        std::vector<vec2> new_points = {p1, p2};
        std::vector<vec2> new_control_point = {c};
        points = new_points;
        control_points = new_control_point;
        points_changed = true;
    }
    void add_segment(vec2 c, vec2 p) {
        points.push_back(p);
        control_points.push_back(c);
        points_changed = true;
    }

    void clear_points() { points.clear(); control_points.clear(); points_changed = true; }
    int get_point_num() { return points.size() + control_points.size(); }
    std::vector<vec2> get_points() { return points; }
    std::vector<vec2> get_control_points() { return control_points; }

    ~BezierLine2D() override {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }
};

#endif 