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
    MatchSlopePathDrawer (Terrain *terrain, float max_slope = 1.f, bool debug_msg = false) 
        : TerrainPathDrawer(terrain,0.f,max_slope,debug_msg) { }   

    void recalculate_path (Line2D* line, vec2 start, vec2 end, float slope) {        
        float step = CONSTANT_SLOPE_PATH_POINT_STEP;
        std::vector<vec2> path_left = terrain->elevation_line_drawer.generate_constant_slope_path(start,end,slope,step);
        line->set_points(path_left);
    }
};

#endif