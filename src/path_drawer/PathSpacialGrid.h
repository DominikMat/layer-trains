// PathCollider.h
struct PathSegmentCollider {
    int id_a; // Handle A ID
    int id_b; // Handle B ID
    std::vector<vec3> points; // The points making up the line
};

class PathSpatialGrid {
    static const int GRID_SIZE = 32; // 32x32 grid
    std::vector<int> grid[GRID_SIZE][GRID_SIZE]; 
    std::vector<PathSegmentCollider> all_paths;

    void get_grid_cell(float x, float y, int &gx, int &gy) {
        // Map -0.5 -> 0.5 range to 0 -> 31
        gx = (int)((x + 0.5f) * GRID_SIZE);
        gy = (int)((y + 0.5f) * GRID_SIZE);
        gx = std::clamp(gx, 0, GRID_SIZE-1);
        gy = std::clamp(gy, 0, GRID_SIZE-1);
    }

public:
    void register_path(int id_a, int id_b, const std::vector<vec3>& points) {
        int index = all_paths.size();
        all_paths.push_back({id_a, id_b, points});

        // Simple rasterization: check start, end, and middle points
        // For robustness, you should walk the line points
        for(auto& p : points) {
            int gx, gy;
            get_grid_cell(p.x, p.y, gx, gy);
            
            // Check if index already added to this cell to avoid duplicates
            bool found = false;
            for(int existing : grid[gx][gy]) if(existing == index) found = true;
            
            if(!found) grid[gx][gy].push_back(index);
        }
    }

    // Returns a pointer to the closest path segment, or nullptr
    // close_point is filled with the exact point on the line
    PathSegmentCollider* get_path_near_mouse(vec3 mouse_local, float radius, vec3& closest_point) {
        int gx, gy;
        get_grid_cell(mouse_local.x, mouse_local.y, gx, gy);

        float min_dist = radius;
        PathSegmentCollider* nearest = nullptr;

        // Check paths in this cell
        for (int path_idx : grid[gx][gy]) {
            PathSegmentCollider& segment = all_paths[path_idx];
            
            // Iterate points in this segment (Linear check is fine here because N is small per cell)
            for (size_t k = 0; k < segment.points.size() - 1; k++) {
                vec3 point_on_seg = MathUtils::closest_point_on_segment(
                    mouse_local, segment.points[k], segment.points[k+1]
                );
                
                float d = glm::distance(mouse_local, point_on_seg);
                if (d < min_dist) {
                    min_dist = d;
                    nearest = &segment;
                    closest_point = point_on_seg;
                }
            }
        }
        return nearest;
    }
};