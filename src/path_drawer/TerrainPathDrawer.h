#ifndef TERRAINPATHDRAWER_H
#define TERRAINPATHDRAWER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "Terrain.h"
#include "InputHandler.h"
#include "PathSystem.h"

using namespace glm;
using namespace std;

class TerrainPathDrawer
{
public:
    vec3 origin_point;
    bool drawing_path = false, debug_msg = false;
    float slope = 0.f;
    float max_slope = 0.15f;
    float last_scroll_value = 1.f;

    int start_draw_handle;

    Terrain *terrain;
    TerrainLine *current_line;

    TerrainPathDrawer (Terrain *terrain, float slope, float max_slope = 0.15f, bool debug_msg = false) 
        : terrain(terrain), debug_msg(debug_msg), slope(slope), max_slope(max_slope) {
        
        current_line = new TerrainLine(terrain->terrain_data);
        current_line->set_parent(terrain->terrain_obj);
    }   

    virtual void update_path (InputHandler *input_handler) {
        /* track scroll value */
        float scroll_value = input_handler->get_scroll_value();
        float delta_scroll = scroll_value - last_scroll_value;
        last_scroll_value = scroll_value;
        
        /* drawing functions - modify slope  */
        if (!drawing_path) return;
        slope = glm::clamp(-max_slope, slope + delta_scroll*PATH_DRAW_SLOPE_CHANGE_SPEED, max_slope);
        
        /* draw path to mouse terrain pos */
        vec3 mouse_pos_world = input_handler->get_mouse_position_world();
        vec4 local_pos_4 = glm::inverse(terrain->terrain_obj->get_transform()) * vec4(mouse_pos_world, 1.f);
        vec3 mouse_pos_local = vec3(local_pos_4);

        if (abs(mouse_pos_local.x) > 0.5f || abs(mouse_pos_local.y) > 0.5f) return;

        recalculate_path(current_line, origin_point, mouse_pos_local, slope);

    }
    
    virtual bool start_drawing_at_pos (vec3 local_pos, int handle_id) {
        if (abs(local_pos.x) > 0.5f || abs(local_pos.y) > 0.5f) return false;
        
        start_draw_handle = handle_id;
        drawing_path = true;
        origin_point = local_pos;            
        if (debug_msg) std::cout << "Path started." << std::endl;
        return true;
    }
    
    virtual bool end_drawing_at_pos (vec3 local_pos) {
        if (abs(local_pos.x) > 0.5f || abs(local_pos.y) > 0.5f) return false;
        
        drawing_path = false;
        recalculate_path(current_line, origin_point, local_pos, slope);

        if (debug_msg) std::cout << (current_line->get_point_num() > 1 ? "Path set." : "Path empty") << std::endl;
        return true;
    }
    TerrainLinkData create_terrain_link(int end_handle_id) {
        TerrainLinkData new_link = {
            start_draw_handle, end_handle_id, current_line->get_points()
        };
        current_line->clear_points();
        return new_link;
    }

    virtual void recalculate_path(Line2D* line, vec2 start, vec2 end, float slope_value=0.f) = 0;

    void reset() { current_line->clear_points(); drawing_path = false; }
    void clear_path() { current_line->clear_points(); }
    bool is_drawing_path() { return drawing_path; }
    vec2 get_end_point() { return current_line->get_last_point(); }
    void set_slope(float slope) { this->slope = slope; }
    void set_max_slope(float max_slope) { this->max_slope = max_slope; }
    float get_slope() { return slope; }
    float get_max_slope() { return max_slope; }

    ~TerrainPathDrawer() { clear_path();  }
};

#endif