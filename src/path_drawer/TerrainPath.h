#ifndef TERRAINPATH_H
#define TERRAINPATH_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "Terrain.h"
#include "InputHandler.h"
#include "Line.h"

using namespace glm;
using namespace std;

class TerrainPath
{
public:
    vec3 origin_point;
    vec3 end_point;
    bool is_drawing_path = false;
    float current_grade = 0.f;
    float max_grade = 15.f;
    Terrain *terrain;
    float scroll_speed_before = 0.f;
    bool debug_msg;

    vector<Line*> lines;
    Line *current_line_segment = nullptr;

    World *w;

    bool modify_scroll_on_next_update = false;

    TerrainPath (Terrain *terrain, World *w, float max_grade = 15.f, bool debug_msg = false) 
        : terrain(terrain), max_grade(max_grade), debug_msg(debug_msg), w(w){
        lines.clear();
    }   

    void change_grade (float new_grade) {
        current_grade = new_grade;

    }

    void update_path (InputHandler *input_handler) {
        vec3 mouse_pos_world = input_handler->get_mouse_position_world();
        vec4 local_pos_4 = glm::inverse(terrain->terrain_obj.get_transform()) * vec4(mouse_pos_world, 1.f);
        vec3 mouse_pos_local = vec3(local_pos_4);

        if (modify_scroll_on_next_update) {
            modify_scroll_on_next_update = false;
            if (is_drawing_path) {
                scroll_speed_before = input_handler->get_scroll_speed();
                input_handler->reset_scroll_value();
                input_handler->set_scroll_speed( PATH_DRAW_GRADE_CHANGE_SPEED );
            } else {
                input_handler->reset_scroll_value();
                input_handler->set_scroll_speed( scroll_speed_before );
            }
        }
        if (is_drawing_path) {
            // modify grade
            float prev_grade = current_grade;
            current_grade = (input_handler->get_scroll_value()-1.f) * max_grade;

            // Update path to mouse
            if (abs(mouse_pos_local.x) <= 0.5f && abs(mouse_pos_local.y) <= 0.5f) {
                std::vector<vec3> segment = terrain->elevation_line_drawer.get_active_path_segment(
                    origin_point, mouse_pos_local, current_grade, prev_grade != current_grade);
                
                // Lift line slightly to avoid Z-fighting (Local Z up)
                for(auto& p : segment) p.z += 0.005f; 
                
                current_line_segment->set_points(segment);
            }

        }
    }

    float get_current_grade() {
        return current_grade;
    }
    bool drawing() {
        return is_drawing_path;
    }

    void start_drawing_at_pos (vec3 local_pos) {
        if (abs(local_pos.x) <= 0.5f && abs(local_pos.y) <= 0.5f) {
            is_drawing_path = true;
            modify_scroll_on_next_update = true;
            origin_point = local_pos;

            create_new_line_segment();
            
            terrain->elevation_line_drawer.clear_cache();
            // Force generate initial segment
            std::vector<vec3> segment = terrain->elevation_line_drawer.get_active_path_segment(origin_point, local_pos, current_grade, true);
            current_line_segment->set_points(segment);
            if (debug_msg) std::cout << "Path started." << std::endl;
        }

    }
    
    void end_drawing_at_pos (vec3 local_pos) {
        std::vector<vec3> segment = terrain->elevation_line_drawer.get_active_path_segment(origin_point, local_pos, current_grade, true);
        current_line_segment->set_points(segment);
        
        is_drawing_path = false;
        modify_scroll_on_next_update = true;
        end_point = segment[segment.size()-1];
        
        if (debug_msg) std::cout << (segment.size() > 1 ? "Path set." : "Path empty") << std::endl;
    }

    void create_new_line_segment() {
        lines.push_back(new Line(LINE_THICKNESS));
        Line *l = lines.at(lines.size()-1);
        l->set_colour( CONTOUR_LINE_COLOUR );
        l->set_parent(&terrain->terrain_obj);
        l->move(CONTOUR_LINE_HEGHT_OFFSET);
        w->place(l);
        current_line_segment = l;
    }

    void clear_path() {
        current_line_segment->clear_points();
        for (auto line_ptr : lines) { delete line_ptr; }
        lines.clear();
        current_line_segment = nullptr;
    }

    ~TerrainPath() { clear_path();  }
};

#endif