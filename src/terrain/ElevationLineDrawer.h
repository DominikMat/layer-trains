#ifndef ELEVATIONLINEDRAWER_H
#define ELEVATIONLINEDRAWER_H

#include "textures/Texture.h"
#include "settings/Settings.h"
#include "Heightmap.h" 
#include "world_objects/Line.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <cfloat>

// Helper for high-precision math
#define PI 3.14159265359f

class ElevationLineDrawer
{
private:
    float heightmap_scale;
    unsigned char* height_data;
    int hmap_width, hmap_height;
    bool height_data_loaded = false;

    // Cache the full valid contour path for the current drawing session
    std::vector<glm::vec3> cached_full_path; 

public:
    ElevationLineDrawer(const char* heightmap_path, float heightmap_scale) : heightmap_scale(heightmap_scale) {
        stbi_set_flip_vertically_on_load(true); 
        int width, height, nrChannels;
        height_data = stbi_load(heightmap_path, &width, &height, &nrChannels, 1);
        hmap_width = width; hmap_height = height;

        if (!height_data) {
            std::cerr << "ERROR: Failed to load heightmap data." << std::endl;
        } else {
            height_data_loaded = true;
        }
    }

    ~ElevationLineDrawer() {
        if (height_data_loaded && height_data) {
            stbi_image_free(height_data);
        }
    }

    // --- Core Algorithm ---
    
    // Trace a single direction (used internally)
    void trace_single_direction(glm::vec2 start_pixels, float grade, float max_dist_pixels, bool reverse_polarity, std::vector<glm::vec3>& out_path) {
        glm::vec2 current_pos = start_pixels;
        float current_dist = 0.f;
        glm::vec2 last_dir = glm::vec2(0,0);
        float step_size = 1.0f; // Step in pixels

        while (current_dist < max_dist_pixels) {
            // 1. Get Gradient
            glm::vec2 gradient = get_gradient_sobel(current_pos);
            if (glm::length(gradient) < 0.0001f) gradient = glm::vec2(0, 1);
            else gradient = glm::normalize(gradient);

            // 2. Contour Vector (Perpendicular). 
            glm::vec2 contour_dir = reverse_polarity ? glm::vec2(gradient.y, -gradient.x) : glm::vec2(-gradient.y, gradient.x);

            // 3. Apply Grade
            float angle = std::atan(grade); 
            glm::vec2 move_dir = contour_dir * std::cos(angle) + gradient * std::sin(angle);
            move_dir = glm::normalize(move_dir);

            // 4. Continuity check
            if (out_path.size() > 1 && glm::length(last_dir) > 0 && glm::dot(move_dir, last_dir) < 0) {
                 break; 
            }

            // 5. Step
            current_pos += move_dir * step_size;

            // Bounds check (keep slightly inside to avoid bilinear sampling errors)
            if (current_pos.x < 1.0f || current_pos.x >= hmap_width - 1.0f || 
                current_pos.y < 1.0f || current_pos.y >= hmap_height - 1.0f) break;

            out_path.push_back(pixel_to_local(current_pos));
            
            last_dir = move_dir;
            current_dist += step_size;
        }
    }

    // Generates a bidirectional path centered at start_pos_local
    std::vector<glm::vec3> generate_bidirectional_path(glm::vec3 start_pos_local, float grade, float max_length_pixels) {
        std::vector<glm::vec3> full_path;
        
        // Ensure start pos is clamped to local bounds [-0.5, 0.5] before converting to pixels
        start_pos_local.x = glm::clamp(start_pos_local.x, -0.5f, 0.5f);
        start_pos_local.y = glm::clamp(start_pos_local.y, -0.5f, 0.5f);

        glm::vec2 start_pixels = local_to_pixel(start_pos_local);

        // 1. Trace "Forward" 
        std::vector<glm::vec3> path_fwd;
        trace_single_direction(start_pixels, grade, max_length_pixels, false, path_fwd);

        // 2. Trace "Backward" 
        std::vector<glm::vec3> path_bwd;
        trace_single_direction(start_pixels, grade, max_length_pixels, true, path_bwd);

        // 3. Merge: [Reversed Backward] -> [Start] -> [Forward]
        std::reverse(path_bwd.begin(), path_bwd.end());
        
        full_path.insert(full_path.end(), path_bwd.begin(), path_bwd.end());
        full_path.push_back(start_pos_local); // Explicitly add the exact start point
        full_path.insert(full_path.end(), path_fwd.begin(), path_fwd.end());

        return full_path;
    }

    std::vector<glm::vec3> get_active_path_segment(glm::vec3 start_pos_local, glm::vec3 cursor_pos_local, float grade = 0.f, bool force_recalc = false) {
        
        // Recalculate the "Rail" if needed
        if (cached_full_path.empty() || force_recalc) {
            // Approx 1000 pixels length trace
            cached_full_path = generate_bidirectional_path(start_pos_local, grade, 2000.f); 
        }

        if (cached_full_path.empty()) return {};

        // Find indices in the cached path
        float min_dist_cursor = FLT_MAX;
        float min_dist_start = FLT_MAX;
        int closest_idx = -1;
        int start_idx = -1;

        for(int i=0; i<cached_full_path.size(); i++) {
            float d_cursor = glm::distance(cached_full_path[i], cursor_pos_local);
            if (d_cursor < min_dist_cursor) { min_dist_cursor = d_cursor; closest_idx = i; }

            // Re-find start index (it's in the middle of the array)
            float d_start = glm::distance(cached_full_path[i], start_pos_local);
            if (d_start < min_dist_start) { min_dist_start = d_start; start_idx = i; }
        }

        std::vector<glm::vec3> segment;
        if (start_idx == -1 || closest_idx == -1) return segment;

        // Extract segment
        if (start_idx <= closest_idx) {
            for (int i = start_idx; i <= closest_idx; i++) segment.push_back(cached_full_path[i]);
        } else {
            for (int i = start_idx; i >= closest_idx; i--) segment.push_back(cached_full_path[i]);
        }

        return segment;
    }

    void clear_cache() {
        cached_full_path.clear();
    }
    
    // --- Public access for Terrain class ---
    // Returns LOCAL position [-0.5, 0.5] relative to plane center
    glm::vec3 get_local_pos_from_uv(float u, float v) {
        float px = u * hmap_width;
        float py = v * hmap_height;
        return pixel_to_local(glm::vec2(px, py));
    }

private:
    // --- Helpers ---

    float get_height_bilinear(float x, float y) {
        int x0 = (int)x; int y0 = (int)y;
        int x1 = std::min(x0 + 1, hmap_width - 1);
        int y1 = std::min(y0 + 1, hmap_height - 1);
        float sx = x - (float)x0;
        float sy = y - (float)y0;
        float h00 = get_raw_height(x0, y0);
        float h10 = get_raw_height(x1, y0);
        float h01 = get_raw_height(x0, y1);
        float h11 = get_raw_height(x1, y1);
        float h0 = glm::mix(h00, h10, sx);
        float h1 = glm::mix(h01, h11, sx);
        return glm::mix(h0, h1, sy);
    }

    float get_raw_height(int x, int y) {
        if (x < 0 || x >= hmap_width || y < 0 || y >= hmap_height) return 0.0f;
        return (float)height_data[y * hmap_width + x] / 255.0f;
    }

    glm::vec2 get_gradient_sobel(glm::vec2 pos) {
        float h_l = get_height_bilinear(pos.x - 1, pos.y);
        float h_r = get_height_bilinear(pos.x + 1, pos.y);
        float h_d = get_height_bilinear(pos.x, pos.y - 1);
        float h_u = get_height_bilinear(pos.x, pos.y + 1);
        return glm::vec2(h_r - h_l, h_u - h_d);
    }

    glm::vec2 local_to_pixel(glm::vec3 local_pos) {
        // Map [-0.5, 0.5] to [0, width]
        float x_norm = (local_pos.x) + 0.5f; 
        float y_norm = (local_pos.y) + 0.5f;
        return glm::vec2(x_norm * hmap_width, y_norm * hmap_height);
    }

    glm::vec3 pixel_to_local(glm::vec2 pixel_pos) {
        float x_norm = pixel_pos.x / (float)hmap_width;
        float y_norm = pixel_pos.y / (float)hmap_height;
        
        // Sample height (0.0 to 1.0)
        float height_norm = get_height_bilinear(pixel_pos.x, pixel_pos.y);
        
        // Map 0-1 back to Local Plane Coordinates [-0.5, 0.5]
        float x_local = x_norm - 0.5f;
        float y_local = y_norm - 0.5f;
        
        // IMPORTANT: We do NOT multiply by World Scale here if the object is a child of the terrain.
        // We only multiply by the heightmap intensity scale relative to the plane width.
        // Assuming hm.scale represents the Z-height relative to a unit plane:
        float z_local = height_norm * heightmap_scale; 

        return glm::vec3(x_local, y_local, z_local);
    }
};

#endif