#ifndef AUTOSLOPEPATH_H
#define AUTOSLOPEPATH_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "Terrain.h"
#include "InputHandler.h"
#include "TerrainPathDrawer.h"
#include "Line2D.h"
#include "World.h"

using namespace glm;
using namespace std;

#define CONSTANT_SLOPE_PATH_POINT_STEP 0.01f;

class AutoSlopePathDrawer : public TerrainPathDrawer
{
public:
    float max_slope = 15.f;

    float scroll_speed_before = 0.f;
    float last_scroll_value = 1.f;
    bool modify_scroll_on_next_update = false;

    AutoSlopePathDrawer (Terrain *terrain, World *w, float max_slope = 1.f, bool debug_msg = false) 
        : TerrainPathDrawer(terrain,w,max_slope,debug_msg), max_slope(max_slope) { }   

    void update_path (InputHandler *input_handler) override {
        /* modify max slope */
        float delta_scroll = input_handler->get_scroll_value() - last_scroll_value;
        last_scroll_value = input_handler->get_scroll_value();
        float prev_slope = max_slope;
        max_slope = glm::clamp(-1.f, max_slope + delta_scroll*PATH_DRAW_SLOPE_CHANGE_SPEED, 1.f);
        set_slope(max_slope);

        TerrainPathDrawer::update_path(input_handler);
    }

    void recalculate_path (Line2D* line, vec2 start, vec2 end, float max_slope) override{        
        float step = CONSTANT_SLOPE_PATH_POINT_STEP;
        std::vector<vec2> path_left = terrain->elevation_line_drawer.generate_auto_slope_path(start,end,max_slope,step);
        line->set_points(path_left);
    }
    
    /* Slope */
    void change_max_slope (float new_slope) { max_slope = new_slope; }
    float get_current_max_slope() { return max_slope; }
    
};

#endif