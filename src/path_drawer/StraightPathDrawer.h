#ifndef STRAIGHTPATH_H
#define STRAIGHTPATH_H

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

#define STRAIGHT_PATH_MINIMUM_TERRAIN_STEP 0.005f

class StraightPathDrawer : public TerrainPathDrawer
{
public:
    StraightPathDrawer (Terrain *terrain, World *w, bool debug_msg = false) 
        : TerrainPathDrawer(terrain,w,0.f,debug_msg) { }   

    void recalculate_path (Line* line, vec3 start, vec3 end, float slope) override {
        float path_dist = glm::length(end-start);
        int point_num = (int)(path_dist / STRAIGHT_PATH_MINIMUM_TERRAIN_STEP) + 2;
        std::vector<vec3> segment; segment.resize(point_num);

        segment[0] = start;
        for (int i=1; i<point_num-1; i++) {
            float t = (float)i / (point_num-1);
            segment[i] = vec3(end.x*t + start.x*(1.f-t), end.y*t + start.y*(1.f-t), 0.f);
            segment[i].z = terrain->elevation_line_drawer.get_height_at_local_pos(segment[i].x,segment[i].y);
        }
        segment[point_num-1] = end;
        line->set_points(segment);
    }
};

#endif