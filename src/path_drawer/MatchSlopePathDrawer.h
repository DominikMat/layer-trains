#ifndef MATCHSLOPEPATH_H
#define MATCHSLOPEPATH_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "Terrain.h"
#include "InputHandler.h"
#include "TerrainPathDrawer.h"
#include "Line.h"
#include "World.h"

using namespace glm;
using namespace std;

#define CONSTANT_SLOPE_PATH_POINT_STEP 0.01f;

class MatchSlopePathDrawer : public TerrainPathDrawer
{
public:
    float current_slope = 0.f;
    float max_slope = 15.f;

    float scroll_speed_before = 0.f;
    float last_scroll_value = 1.f;
    bool modify_scroll_on_next_update = false;

    MatchSlopePathDrawer (Terrain *terrain, World *w, float max_slope = 1.f, bool debug_msg = false) 
        : TerrainPathDrawer(terrain,w,0.f,debug_msg), max_slope(max_slope) { }   

    void update_path (InputHandler *input_handler) override {
        /* modify slope */
        float delta_scroll = input_handler->get_scroll_value() - last_scroll_value;
        last_scroll_value = input_handler->get_scroll_value();
        float prev_slope = current_slope;
        current_slope = glm::clamp(-max_slope, current_slope + delta_scroll*PATH_DRAW_SLOPE_CHANGE_SPEED, max_slope);
        set_slope(current_slope);

        TerrainPathDrawer::update_path(input_handler);
    }

    void recalculate_path (vec3 start, vec3 end, float slope) {        
        // from a given point we can go two direction from any point along the gradient, we generate both and check 
        // which has end closer to the end position
        float step = CONSTANT_SLOPE_PATH_POINT_STEP;
        std::vector<vec3> path_left = terrain->elevation_line_drawer.generate_constant_slope_path(start,end,slope,step);
        //std::vector<vec3> path_right = terrain->elevation_line_drawer.generate_constant_slope_path(start,end,slope,step,false);

        // choose better path
        // float dist_left = glm::length(path_left.at(path_left.size()-1) - end);
        // float dist_right = glm::length(path_right.at(path_right.size()-1) - end);
        // current_line_segment->set_points(dist_left < dist_right ? path_left : path_right);
        current_line_segment->set_points(path_left);
    }
    
    /* Slope */
    void change_max_slope (float new_slope) { max_slope = new_slope; }
    float get_current_slope() { return current_slope; }
    
};



    // void update_path (InputHandler *input_handler) override {
    //     vec3 mouse_pos_world = input_handler->get_mouse_position_world();
    //     vec4 local_pos_4 = glm::inverse(terrain->terrain_obj.get_transform()) * vec4(mouse_pos_world, 1.f);
    //     vec3 mouse_pos_local = vec3(local_pos_4);

    //     if (modify_scroll_on_next_update) {
    //         modify_scroll_on_next_update = false;
    //         if (drawing_path) {
    //             scroll_speed_before = input_handler->get_scroll_speed();
    //             input_handler->set_scroll_speed( PATH_DRAW_SLOPE_CHANGE_SPEED );
    //             last_scroll_value = input_handler->get_scroll_value();
    //         } else {
    //             input_handler->set_scroll_speed( scroll_speed_before );
    //         }
    //     }
    //     if (drawing_path) {
            
    //         /* modify slope */
    //         float delta_scroll = input_handler->get_scroll_value() - last_scroll_value;
    //         last_scroll_value = input_handler->get_scroll_value();
    //         float prev_slope = current_slope;
    //         current_slope = glm::clamp(-max_slope, current_slope + delta_scroll*PATH_DRAW_SLOPE_CHANGE_SPEED, max_slope);

    //         // Update path to mouse
    //         if (abs(mouse_pos_local.x) <= 0.5f && abs(mouse_pos_local.y) <= 0.5f) {
    //             std::vector<vec3> segment = terrain->elevation_line_drawer.get_active_path_segment(
    //                 origin_point, mouse_pos_local, current_slope, prev_slope != current_slope);
                
    //             // Lift line slightly to avoid Z-fighting (Local Z up)
    //             for(auto& p : segment) p.z += 0.005f; 
                
    //             current_line_segment->set_points(segment);
    //         }

    //     }
    // }

    // void start_drawing_at_pos (vec3 local_pos) override {
    //     if (abs(local_pos.x) <= 0.5f && abs(local_pos.y) <= 0.5f) {
    //         drawing_path = true;
    //         modify_scroll_on_next_update = true;
    //         origin_point = local_pos;

    //         create_new_line_segment();
            
    //         terrain->elevation_line_drawer.clear_cache();
    //         // Force generate initial segment
    //         std::vector<vec3> segment = terrain->elevation_line_drawer.get_active_path_segment(origin_point, local_pos, current_slope, true);
    //         current_line_segment->set_points(segment);
    //         if (debug_msg) std::cout << "Path started." << std::endl;
    //     }

    // }
    
    // void end_drawing_at_pos (vec3 local_pos) override {
    //     std::vector<vec3> segment = terrain->elevation_line_drawer.get_active_path_segment(origin_point, local_pos, current_slope, true);
    //     current_line_segment->set_points(segment);
        
    //     drawing_path = false;
    //     modify_scroll_on_next_update = true;
    //     end_point = segment[segment.size()-1];
        
    //     if (debug_msg) std::cout << (segment.size() > 1 ? "Path set." : "Path empty") << std::endl;
    // }

    // /* Slope */
    // void change_slope (float new_slope) { current_slope = new_slope; }
    // float get_current_slope() { return current_slope; }

#endif