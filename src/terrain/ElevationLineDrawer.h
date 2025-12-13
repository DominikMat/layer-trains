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
using namespace glm;
using namespace std;

struct CachedPathData {
    vec2 start, end;
    float slope, step;
    int mode; // 0 - match slope, 1 - auto
};

class ElevationLineDrawer
{
private:
    float heightmap_scale;
    void* height_data; 
    bool is_16bit_data;
    int hmap_width, hmap_height;
    bool height_data_loaded = false;

    vector<vec3> cached_path;
    CachedPathData cached_path_data;

public:
    ElevationLineDrawer(const char* heightmap_path, float heightmap_scale, bool use_16bit = false) 
        : heightmap_scale(heightmap_scale), is_16bit_data(use_16bit) 
    {
        stbi_set_flip_vertically_on_load(true); 
        int width, height, nrChannels;
        if (is_16bit_data) {
            height_data = stbi_load_16(heightmap_path, &width, &height, &nrChannels, 1);
        } else {
            height_data = stbi_load(heightmap_path, &width, &height, &nrChannels, 1);
        }
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

    /* uv is [0,1] terrain position, these function return local position and height  */
    float get_height_at_uv(float u, float v) {
        return get_height_bilinear(u*hmap_width,v*hmap_height) * heightmap_scale;
    }
    glm::vec3 get_local_pos_from_uv(float u, float v) {
        u = glm::clamp(u,0.f,1.f); v = glm::clamp(v,0.f,1.f);
        return vec3(u-.5f,v-.5f,get_height_at_uv(u,v));
    }

    /* local position is [-.5,.5] terrain position, these function return local position and height  */
    float get_height_at_local_pos(float x, float y) {
        if (x<-0.5f || y<-0.5f || x>0.5f || y>0.5f) return 0.f;
        return get_height_bilinear((x+0.5f)*hmap_width,(y+0.5f)*hmap_height) * heightmap_scale;
    }
    glm::vec2 local_to_uv(glm::vec2 local) { return glm::vec2(local.x-0.5f,local.y-0.5f); }

    /* Line drawing algorithm */
    void clear_cache() { cached_path.clear(); }
    vector<vec3> generate_constant_slope_path(vec3 start, vec2 end, float slope, float step, bool direction = true) {
        
        if (!cached_path.empty() && 
            distance(vec2(start), cached_path_data.start) < 0.001f && 
            distance(end, cached_path_data.end) < 0.05f && 
            abs(slope - cached_path_data.slope) < 0.001f &&
            abs(step - cached_path_data.step) < 0.001f  &&
            cached_path_data.mode == 0 ) { 
            return cached_path;
        }
        
        cached_path_data = { vec2(start), end, slope, step, 0 };
        cached_path.clear();
        cached_path.push_back(start);

        int max_safety_steps = 2000;
        float min_dist_to_end = length(end - vec2(start));
        int min_dist_index = 0;

        for(int i = 0; i < max_safety_steps; i++) {
            /* get target direction */
            vec2 end_dir = (end-vec2(cached_path.back()));
            vec2 end_dir_normalise = end_dir / length(end_dir);
            
            /* add new point */
            float target_height = cached_path.back().z + step*slope;
            vec2 next_point = follow_slope_gradient(vec2(cached_path.back()) + end_dir_normalise*step, target_height);
            cached_path.push_back( vec3(next_point, get_height_at_local_pos(next_point.x,next_point.y)) );

            /* calculate final distances */
            float dist = length(end-vec2(cached_path.back()));
            float points_dist = length(vec2(cached_path.back())-vec2(cached_path[cached_path.size()-2]));
            if (dist < min_dist_to_end) {
                min_dist_to_end = dist;
                min_dist_index = cached_path.size()-1;
            }

            /* exit conditions */
            if (points_dist < 0.5*step) break;
            if (dist > min_dist_to_end + step*20.f) break; // heuristic
            if (dist < step) break;
        }

        /* Path smoothing */
        if (cached_path.size() > 2) {
            vector<vec3> smoothed = cached_path;
            for(int i = 1; i < cached_path.size() - 1; i++) {
                smoothed[i] = (cached_path[i-1] + cached_path[i] + cached_path[i+1]) / 3.0f;
            }
            cached_path = smoothed;
        }

        if (min_dist_index < cached_path.size() - 1) cached_path.resize(min_dist_index+1);
        return cached_path;
    }
    vector<vec3> generate_auto_slope_path(vec3 start, vec2 end, float max_slope, float step, bool direction = true) {
        
        if (!cached_path.empty() && 
            distance(vec2(start), cached_path_data.start) < 0.001f && 
            distance(end, cached_path_data.end) < 0.05f && 
            abs(max_slope - cached_path_data.slope) < 0.001f &&
            abs(step - cached_path_data.step) < 0.001f  && 
            cached_path_data.mode == 1 ) { 
            return cached_path;
        }
        
        cached_path_data = { vec2(start), end, max_slope, step, 1 };
        cached_path.clear();
        cached_path.push_back(start);

        // Calculate automatic slope
        float end_z = get_height_at_local_pos(end.x, end.y);
        float total_dist = length(end - vec2(start));
        float needed_slope = (end_z - start.z) / (total_dist > 0.001f ? total_dist : 1.f);
        float actual_slope = glm::clamp(needed_slope, -max_slope, max_slope);

        int max_safety_steps = 2000;
        float min_dist_to_end = length(end - vec2(start));
        int min_dist_index = 0;

        for(int i = 0; i < max_safety_steps; i++) {
            /* get target direction */
            vec2 end_dir = (end-vec2(cached_path.back()));
            vec2 end_dir_normalise = end_dir / length(end_dir);
            
            /* add new point */
            float target_height = cached_path.back().z + step*actual_slope;
            vec2 next_point = follow_slope_gradient(vec2(cached_path.back()) + end_dir_normalise*step, target_height);
            cached_path.push_back( vec3(next_point, get_height_at_local_pos(next_point.x,next_point.y)) );

            /* calculate final distances */
            float dist = length(end-vec2(cached_path.back()));
            float points_dist = length(vec2(cached_path.back())-vec2(cached_path[cached_path.size()-2]));
            if (dist < min_dist_to_end) {
                min_dist_to_end = dist;
                min_dist_index = cached_path.size()-1;
            }

            /* exit conditions */
            if (points_dist < 0.5*step) break;
            if (dist > min_dist_to_end + step*20.f) break; // heuristic
            if (dist < step) break;
        }

        /* Path smoothing */
        if (cached_path.size() > 2) {
            vector<vec3> smoothed = cached_path;
            for(int i = 1; i < cached_path.size() - 1; i++) {
                smoothed[i] = (cached_path[i-1] + cached_path[i] + cached_path[i+1]) / 3.0f;
            }
            cached_path = smoothed;
        }

        //points.pop_back(); // last point was further from end than second to last, remove it
        if (min_dist_index < cached_path.size() - 1) cached_path.resize(min_dist_index+1);
        return cached_path;
    }

private:
    // this function returns the height value for any x,y given in fractional pixel values
    // eg. pixel value of x=1.5f is average height from pixel 1 and pixel 2 together
    float get_height_bilinear(float x, float y) {
        if (!height_data_loaded) return 0.f;
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
        size_t index = (size_t)y * hmap_width + x;

        if (is_16bit_data) {
            // Rzutowanie void* na unsigned short* i odczyt 16-bitowej wartości
            unsigned short* data_16bit = static_cast<unsigned short*>(height_data);
            unsigned short raw_value = data_16bit[index];
            // Normalizacja 16-bitowej wartości (0-65535) do float (0.0-1.0)
            return (float)raw_value / 65535.0f; 
        } else {
            // Rzutowanie void* na unsigned char* i odczyt 8-bitowej wartości
            unsigned char* data_8bit = static_cast<unsigned char*>(height_data);
            unsigned char raw_value = data_8bit[index];
            // Normalizacja 8-bitowej wartości (0-255) do float (0.0-1.0)
            return (float)raw_value / 255.0f;
        }
    }   

    vec2 follow_slope_gradient(vec2 pos, float target_height) {
        const int MAX_ITERATIONS = 8; // Reduced iterations for performance
        const float DISTANCE_EPSILON = 0.001f; 
        float eps = 1.0f / (float)hmap_width; 

        for (int i = 0; i < MAX_ITERATIONS; i++) {
            float current_h = get_height_at_local_pos(pos.x, pos.y);
            float diff = target_height - current_h;

            if (abs(diff) < DISTANCE_EPSILON) return pos;

            float h_x1 = get_height_at_local_pos(pos.x + eps, pos.y);
            float h_x2 = get_height_at_local_pos(pos.x - eps, pos.y);
            float grad_x = (h_x1 - h_x2) / (2.0f * eps);

            float h_y1 = get_height_at_local_pos(pos.x, pos.y + eps);
            float h_y2 = get_height_at_local_pos(pos.x, pos.y - eps);
            float grad_y = (h_y1 - h_y2) / (2.0f * eps);

            vec2 gradient(grad_x, grad_y);
            float grad_len_sq = dot(gradient, gradient);

            if (grad_len_sq < 0.000001f) break; // Flat terrain

            // Limit step size to avoid shooting off into infinity on flat slopes
            vec2 offset = gradient * (diff / grad_len_sq);
            float offset_len = length(offset);
            if(offset_len > 0.05f) offset = (offset / offset_len) * 0.05f; // Cap jump size

            pos += offset;
            pos = clamp(pos, vec2(-0.5f), vec2(0.5f));
        }
        return pos;
    }
    
};

#endif