#ifndef TERRAINPATHDRAWER_H
#define TERRAINPATHDRAWER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "Terrain.h"
#include "InputHandler.h"
#include "TerrainLine.h"

using namespace glm;
using namespace std;

class TerrainPathDrawer
{
public:
    vec3 origin_point;
    bool drawing_path = false, debug_msg = false;
    float slope = 0.f;

    Terrain *terrain;

    TerrainLine *current_line;
    TerrainLine *set_line;

    TerrainPathDrawer (Terrain *terrain, World *w, float slope, bool debug_msg = false) 
        : terrain(terrain), debug_msg(debug_msg), slope(slope) {
        
        current_line = new TerrainLine(terrain->terrain_data);
        //current_line->set_colour( PATH_COLOUR );
        current_line->set_parent(terrain->terrain_obj);
        //current_line->move(CONTOUR_LINE_HEGHT_OFFSET);
        w->place(current_line);

        set_line = new TerrainLine(terrain->terrain_data);
        //set_line->set_colour( PATH_COLOUR );
        set_line->set_parent(terrain->terrain_obj);
        //set_line->move(CONTOUR_LINE_HEGHT_OFFSET);
        w->place(set_line);
    }   

    virtual void update_path (InputHandler *input_handler) {
        if (!drawing_path) return;

        /* draw path to mouse terrain pos */
        vec3 mouse_pos_world = input_handler->get_mouse_position_world();
        vec4 local_pos_4 = glm::inverse(terrain->terrain_obj->get_transform()) * vec4(mouse_pos_world, 1.f);
        vec3 mouse_pos_local = vec3(local_pos_4);

        if (abs(mouse_pos_local.x) > 0.5f || abs(mouse_pos_local.y) > 0.5f) return;

        recalculate_path(current_line, origin_point, mouse_pos_local, slope);

    }
    
    virtual void start_drawing_at_pos (vec3 local_pos) {
        if (abs(local_pos.x) <= 0.5f && abs(local_pos.y) <= 0.5f) {
            drawing_path = true;
            origin_point = local_pos;            
            if (debug_msg) std::cout << "Path started." << std::endl;
        }
        
    }
    
    virtual void end_drawing_at_pos (vec3 local_pos) {
        drawing_path = false;
        recalculate_path(current_line, origin_point, local_pos, slope);

        if (debug_msg) std::cout << (current_line->get_point_num() > 1 ? "Path set." : "Path empty") << std::endl;

        set_line->add_points ( current_line->get_points() );
        current_line->clear_points();
    }

    virtual void recalculate_path(Line* line, vec3 start, vec3 end, float slope_value=0.f) = 0;

    void reset() {
        if (drawing_path) current_line->clear_points();
        drawing_path = false;
    }
    void clear_path() {
        current_line->clear_points();
        set_line->clear_points();
    }
    
    bool is_drawing_path() {
        return drawing_path;
    }

    vec3 get_end_point() {
        return current_line->get_last_point();
    }

    void set_slope(float slope) {
        this->slope = slope;
    }

    ~TerrainPathDrawer() { clear_path();  }
};

#endif