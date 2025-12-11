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
    bool drawing_path = false, debug_msg = false;
    float slope = 0.f;

    Terrain *terrain;
    World *w;

    vector<Line*> lines;
    Line *current_line_segment = nullptr;

    TerrainPath (Terrain *terrain, World *w, float slope, bool debug_msg = false) 
        : terrain(terrain), debug_msg(debug_msg), w(w), slope(slope) {
        lines.clear();
    }   

    virtual void update_path (InputHandler *input_handler) {
        if (!drawing_path) return;

        /* draw path to mouse terrain pos */
        vec3 mouse_pos_world = input_handler->get_mouse_position_world();
        vec4 local_pos_4 = glm::inverse(terrain->terrain_obj->get_transform()) * vec4(mouse_pos_world, 1.f);
        vec3 mouse_pos_local = vec3(local_pos_4);

        if (abs(mouse_pos_local.x) > 0.5f || abs(mouse_pos_local.y) > 0.5f) return;

        recalculate_path(origin_point, mouse_pos_local, slope);

    }
    
    virtual void start_drawing_at_pos (vec3 local_pos) {
        if (abs(local_pos.x) <= 0.5f && abs(local_pos.y) <= 0.5f) {
            drawing_path = true;
            origin_point = local_pos;
            
            create_new_line_segment();
            
            if (debug_msg) std::cout << "Path started." << std::endl;
        }
        
    }
    
    virtual void end_drawing_at_pos (vec3 local_pos) {
        drawing_path = false;
        recalculate_path(origin_point, local_pos, slope);
        
        if (debug_msg) std::cout << (current_line_segment->get_point_num() > 1 ? "Path set." : "Path empty") << std::endl;
    }

    virtual void recalculate_path(vec3 start, vec3 end, float slope_value=0.f) = 0;

    void create_new_line_segment() {
        lines.push_back(new Line(LINE_THICKNESS));
        Line *l = lines.at(lines.size()-1);
        l->set_colour( CONTOUR_LINE_COLOUR );
        l->set_parent(terrain->terrain_obj);
        l->move(CONTOUR_LINE_HEGHT_OFFSET);
        w->place(l);
        current_line_segment = l;
    }

    void reset() {
        if (drawing_path) current_line_segment->clear_points();
        drawing_path = false;
    }
    void clear_path() {
        current_line_segment->clear_points();
        for (auto line_ptr : lines) { delete line_ptr; }
        lines.clear();
        current_line_segment = nullptr;
    }
    
    bool is_drawing_path() {
        return drawing_path;
    }

    vec3 get_end_point() {
        return current_line_segment->get_last_point();
    }

    void set_slope(float slope) {
        this->slope = slope;
    }

    ~TerrainPath() { clear_path();  }
};

#endif