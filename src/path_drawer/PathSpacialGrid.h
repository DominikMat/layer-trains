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
    static const int GRID_SIZE = 32; 
    const float GRID_STEP = 1.f / GRID_SIZE;
    std::vector<GridEntry> grid[GRID_SIZE][GRID_SIZE];

public:
    void clear() {
        for(int x=0; x<GRID_SIZE; x++)
            for(int y=0; y<GRID_SIZE; y++) grid[x][y].clear();
    }

    void register_path_segment(int add_link_id, vector<vec2> points) 
    {    
        // find all distinct grid cells where path points lay
        ivec2 last_grid_point = ivec2(0);
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
        for (int x=0; x<GRID_SIZE; x++) {
            for (int y=0; y<GRID_SIZE; y++) {
                for (int i=0; i<grid[x][y].size(); i++) {
                    int link_id = grid[x][y][i].link_id;
                    if (link_id == remove_link_id) {
                        grid[x][y].erase(grid[x][y].begin()+i);
                    }
                }
            }
        }
    }

    // Returns -1 if no path found, otherwise returns link_index
    int get_link_at_pos(vec2 pos, float radius, vec2& out_closest_point) 
    {
        // check what area of grid need to be checked (minimum is 3x3 because we might be on grid cell edge)
        int search_size = 1 + (int)(radius / GRID_STEP);
        
        ivec2 grid_cell = get_grid_cell(pos);
        float min_dist = radius + 0.00001f;
        int found_link = -1;

        for (int x=grid_cell.x-search_size; x<grid_cell.x+search_size; x++) {
            for (int y=grid_cell.y-search_size; y<grid_cell.y+search_size; y++) { 
                if (x<0 || y<0 || x>=GRID_SIZE || y>=GRID_SIZE) continue;
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
        int gx = (int)((point.x + 0.5f) * GRID_SIZE);
        int gy = (int)((point.y + 0.5f) * GRID_SIZE);
        return ivec2(std::clamp(gx, 0, GRID_SIZE-1), gy = std::clamp(gy, 0, GRID_SIZE-1));
    }
};

#endif