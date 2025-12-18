#ifndef PATHSPATIALGRID_H
#define PATHSPATIALGRID_H

#include "settings/Utility.h"
#include <vector>
#include <algorithm>

using namespace glm;


class PathSpatialGrid {

private:
    struct GridEntry {
        int link_id;
        vec2 exact_position;
    };
    static const int GRID_SIZE_X = 64; 
    static const int GRID_SIZE_Y = GRID_SIZE_X; 
    const float GRID_STEP_X = 1.f / GRID_SIZE_X;
    const float GRID_STEP_Y = GRID_STEP_X;

    std::vector<GridEntry> grid[GRID_SIZE_X][GRID_SIZE_Y];

public:

    PathSpatialGrid () { }

    void clear() {
        for(int x=0; x<GRID_SIZE_X; x++)
            for(int y=0; y<GRID_SIZE_Y; y++) grid[x][y].clear();
    }

    void register_path_segment(int add_link_id, vector<vec2> points) 
    {    
        // find all distinct grid cells where path points lay
        ivec2 last_grid_point = ivec2(-1);
        for (auto p : points) {
            ivec2 grid_pos = get_grid_cell(p);
            if (grid_pos != last_grid_point) {
                GridEntry new_entry = { add_link_id, p };
                grid[grid_pos.x][grid_pos.y].push_back(new_entry);
            }
            last_grid_point = grid_pos;
        }
    }
    void unregister_path_segment(int remove_link_id) 
    {
        // im sorry comuputer , it has to be done 
        // go thru whole grid and remove all idx matching remove_link_id
        for (int x=0; x<GRID_SIZE_X; x++) {
            for (int y=0; y<GRID_SIZE_Y; y++) {
                auto& cell = grid[x][y];
                cell.erase(std::remove_if(cell.begin(), cell.end(), 
                    [remove_link_id](const GridEntry& e) { 
                        return e.link_id == remove_link_id; 
                    }), cell.end());
            }
        }
    }

    // Returns -1 if no path found, otherwise returns link_index
    int get_link_at_pos(vec2 pos, float radius, vec2& out_closest_point) 
    {
        // check what area of grid need to be checked (minimum is 3x3 because we might be on grid cell edge)
        int search_size_x = 1 + (int)(radius / GRID_STEP_X);
        int search_size_y = 1 + (int)(radius / GRID_STEP_Y);
        
        ivec2 grid_cell = get_grid_cell(pos);
        float min_dist = radius + 0.00001f;
        int found_link = -1;

        for (int x=grid_cell.x-search_size_x; x<grid_cell.x+search_size_x; x++) {
            for (int y=grid_cell.y-search_size_y; y<grid_cell.y+search_size_y; y++) { 
                if (x<0 || y<0 || x>=GRID_SIZE_X || y>=GRID_SIZE_Y) continue;
                for (GridEntry entry : grid[x][y]) {
                    float dist = glm::distance(pos, entry.exact_position);
                    if (dist < min_dist) {
                        min_dist = dist;
                        found_link = entry.link_id;
                        out_closest_point = entry.exact_position;
                    }
                }
            }
        }
        return found_link; // will be -1 if no links in range
    }

private:
    ivec2 get_grid_cell(vec2 point) {
        int gx = (int)((point.x + 0.5f) * GRID_SIZE_X);
        int gy = (int)((point.y + 0.5f) * GRID_SIZE_Y);
        return ivec2(std::clamp(gx, 0, GRID_SIZE_X-1), gy = std::clamp(gy, 0, GRID_SIZE_Y-1));
    }
};

#endif