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
    std::vector<int> line_segment_breaks;
    float line_thickness = 5.f;

public:
    Line(std::vector<vec3> points, float line_thickness = 3.f)
        : Object(vec3(0.f), vec3(1.f)), line_thickness(line_thickness),
        points(points)
    { }    

    Line(float line_thickness = 3.f)
        : Object(vec3(0.f), vec3(1.f)), line_thickness(line_thickness),
        points(NULL)
    { }

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
        
        glDrawArrays(GL_LINE_STRIP, 0, line_segment_breaks[0]);
        for (int i=1; i<line_segment_breaks.size(); i++) 
            glDrawArrays(GL_LINE_STRIP, line_segment_breaks[i-1]+1, line_segment_breaks[i]-line_segment_breaks[i-1]-1);
       
        glBindVertexArray(0);
        //glEnable(GL_DEPTH_TEST);
    }

    void add_point(vec3 p) {
        points.push_back(p);
        if(line_segment_breaks.size() == 0) line_segment_breaks.push_back(points.size());
        else line_segment_breaks[line_segment_breaks.size()-1] ++;
    }
    void set_points(std::vector<vec3> new_points) {
        line_segment_breaks.clear();
        for (int i=0; i<new_points.size(); i++) {
            if (new_points[i] == NEW_LINE_SEGMENT_V3) {
                line_segment_breaks.push_back(i);
            }
        }
        this->points = new_points;
        line_segment_breaks.push_back(new_points.size());
    }
    void clear_points() {
        points.clear();
    }

    ~Line() override {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }
};

#endif 

// #ifndef LINE_H
// #define LINE_H

// #include <glad/glad.h>
// #include <GLFW/glfw3.h>

// #include <string>
// #include <fstream>
// #include <sstream>
// #include <iostream>
// #include <vector>
// #include "world_objects/Object.h"

// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/type_ptr.hpp>

// using namespace glm;

// class Line : public Object
// {
// private:
//     std::vector<unsigned int> vao;
//     std::vector<unsigned int> vbo;
//     //std::vector<vec3> points;
//     std::vector<std::vector<vec3>> segmented_points; 
//     int line_segment_num = 0;
//     float line_thickness = 5.f;

// public:
//     Line(std::vector<vec3> points, float line_thickness = 3.f) 
//         : Object(vec3(0.f), vec3(1.f)), line_thickness(line_thickness)
//     { set_points(points); }    

//     Line(float line_thickness = 3.f) 
//         : Object(vec3(0.f), vec3(1.f)), line_thickness(line_thickness)
//     { }

//     void construct() override {
//         for (int i=0; i<line_segment_num; i++) {
//             glGenVertexArrays(1, &vao[i]);
//             glGenBuffers(1, &vbo[i]);
//             glBindVertexArray(vao[i]);
//             glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
//             glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
//             glEnableVertexAttribArray(0);
//             glBindVertexArray(0);
//         }
//     }

//     // render: używa segments, nie indeksów sentinel
//     void render() override {
//         for (int i=0; i<line_segment_num; i++) {

//             if (segmented_points[i].size() < 2) return;
            
//             glDisable(GL_DEPTH_TEST);
//             glBindVertexArray(vao[i]);
//             glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
//             glBufferData(GL_ARRAY_BUFFER, segmented_points[i].size() * sizeof(glm::vec3), segmented_points[i].data(), GL_DYNAMIC_DRAW);
            
//             glLineWidth(line_thickness);
//             glDrawArrays(GL_LINE_STRIP, 0, segmented_points[i].size());
            
//             glBindVertexArray(0);
//             glEnable(GL_DEPTH_TEST);
//         }
//     }

//     void add_point(vec3 p) {
//         if (line_segment_num > 0){
//             segmented_points[line_segment_num-1].push_back(p);
//         }
//     }
//     void set_points(const std::vector<glm::vec3>& new_points, bool multiple_segments = false) {
//         segmented_points.clear();
        
//         if (!multiple_segments) {
//             points = new_points;
//             segments.emplace_back(0, (int)points.size());
//             return;
//         }

//         int current_start = 0;
//         for (size_t i = 0; i < new_points.size(); ++i) {
//             if (new_points[i] == NEW_LINE_SEGMENT_V3) {
//                 // dodaj segment od current_start do i-1 (jeśli są punkty)
//                 int count = (int)i - current_start;
//                 if (count > 0) segments.emplace_back(current_start, count);
//                 // nie kopiujemy sentinel do points --> just skip it
//                 current_start = (int)i + 1;
//             } else {
//                 points.push_back(new_points[i]);
//             }
//         }
//         // pozostały segment (jeśli jakiś)
//         if (current_start < (int)new_points.size()) {
//             // ale pamiętaj: current_start odnosi się do oryginalnego new_points,
//             // ponieważ kopiowaliśmy tylko nie-sentinel, trzeba poprawić starts.
//             // Prostsze: buduj segments na podstawie points przy iteracji:
//             // poniższy kod jest bezpieczniejszy:
//         }

//         // Prostszą i bezbłędną wersją jest:
//         points.clear();
//         segments.clear();
//         int segStart = 0;
//         for (size_t i=0; i<new_points.size(); ++i) {
//             if (new_points[i] == NEW_LINE_SEGMENT_V3) {
//                 int segCount = (int)points.size() - segStart; // liczba punktów dopisanych do current segment
//                 if (segCount > 0) segments.emplace_back(segStart, segCount);
//                 segStart = (int)points.size(); // next segment will start at current compacted size
//             } else {
//                 points.push_back(new_points[i]);
//             }
//         }
//         // ostatni segment
//         if (points.size() > (size_t)segStart) segments.emplace_back(segStart, (int)points.size() - segStart);
//     }


//     ~Line() override {
//         glDeleteVertexArrays(1, &vao);
//         glDeleteBuffers(1, &vbo);
//     }
// };

// #endif