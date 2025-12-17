#ifndef TUNNELPATH_H
#define TUNNELPATH_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "Terrain.h"
#include "InputHandler.h"
#include "TerrainPathDrawer.h"
#include "TunnelLine.h"
#include "Line.h"
#include "World.h"

using namespace glm;
using namespace std;

#define STRAIGHT_PATH_MINIMUM_TERRAIN_STEP 0.005f

class TunnelPathDrawer : public TerrainPathDrawer
{
    Terrain *t;
    ElevationLineDrawer *elevation_data;

public:
    TunnelPathDrawer (Terrain *terrain, bool debug_msg = false) 
        : TerrainPathDrawer(terrain,0.f,debug_msg), t(terrain) { 
            elevation_data = &terrain->elevation_line_drawer;
        }   

    Line2D* create_line_obj() override {
        Line2D* new_line = new TunnelLine(terrain->terrain_data); 
        new_line->set_parent(terrain->terrain_obj);
        return new_line;
    }

    void recalculate_path (Line2D* line, vec2 start, vec2 end, float slope) override {
        /* check if straight line from start to end doesnt intersect any terrain */
        const float line_minimum_step = 0.001f;
        float line_dist = glm::distance(start,end);
        int line_check_num = line_dist / line_minimum_step;

        float start_h = elevation_data->get_height_at_local_pos(start);
        float end_h = elevation_data->get_height_at_local_pos(end);

        vec2 last_point = start;

        for (int i=1; i<line_check_num-1; i++){
            float t = ((float)i / (line_check_num-1));
            vec2 along_line_pos = start*(1-t) + end*t; // lerp
            float line_h = start_h + (end_h-start_h)*t;
            float terrain_h = elevation_data->get_height_at_local_pos(along_line_pos);

            if (line_h >= terrain_h) break;
            last_point = (i == line_check_num-2) ? end : along_line_pos; // snap to end pos in last iteration
        }

        /* apply exact path from start to end */
        std::vector<vec2> segment = { start, last_point };
        line->set_points(segment);
    }
};

#endif