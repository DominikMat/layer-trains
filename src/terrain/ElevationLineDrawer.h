#ifndef ELEVATIONLINEDRAWER_H
#define ELEVATIONLINEDRAWER_H

#define GLM_ENABLE_EXPERIMENTAL

#include "textures/Texture.h"
#include "settings/Settings.h"
#include "Heightmap.h" 
#include <vector>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp> 
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
    // Gradient data removed - not needed for sampling approach
    bool is_16bit_data;
    int hmap_width, hmap_height;
    bool height_data_loaded = false;

    vector<vec2> cached_path;
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
            return;
        } else {
            height_data_loaded = true;
        }
        // Removed load_gradient() call
    }

    ~ElevationLineDrawer() {
        if (height_data_loaded && height_data) {
            stbi_image_free(height_data);
        }
    }

    float get_height_at_uv(float u, float v) {
        return get_height_bilinear(u*hmap_width,v*hmap_height) * heightmap_scale;
    }

    /* local position is [-.5,.5] terrain position */
    float get_height_at_local_pos(float x, float y) {
        if (x<-0.5f || y<-0.5f || x>0.5f || y>0.5f) return 0.f;
        return get_height_bilinear((x+0.5f)*hmap_width,(y+0.5f)*hmap_height) * heightmap_scale;
    }
    float get_height_at_local_pos(vec2 p) { return get_height_at_local_pos(p.x,p.y); }
    
    glm::vec3 get_local_pos_from_uv(float u, float v) {
        u = glm::clamp(u,0.f,1.f); v = glm::clamp(v,0.f,1.f);
        return vec3(u-.5f,v-.5f,get_height_at_uv(u,v));
    }

    /* Line drawing algorithm */
    void clear_cache() { cached_path.clear(); }
    
    // ==========================================================
    // MODE 1: CONSTANT SLOPE
    // Tries to find a path where dH/dist is constant
    // ==========================================================
vector<vec2> generate_constant_slope_path(vec2 start, vec2 end, float target_slope, float step, bool direction = true) {
        
        if (!cached_path.empty() && 
            distance(start, cached_path_data.start) < 0.001f && 
            distance(end, cached_path_data.end) < 0.05f && 
            abs(target_slope - cached_path_data.slope) < 0.001f &&
            abs(step - cached_path_data.step) < 0.001f  &&
            cached_path_data.mode == 0 ) { 
            return cached_path;
        }
        
        cached_path_data = { start, end, target_slope, step, 0 };
        cached_path.clear();
        cached_path.push_back(start);

        vec2 current_pos = start;
        const int MAX_STEPS = 2000;
        
        // Increased samples for precision finding the "perfect" exit angle
        const int SAMPLES = 32; 
        const float MAX_ANGLE = radians(85.0f); 
        
        // STRICTNESS SETTING: 
        // Max allowed deviation from target slope. 
        // e.g., if target is 0.10, allows 0.08 to 0.12
        const float SLOPE_TOLERANCE = 0.08f; 

        for(int i = 0; i < MAX_STEPS; i++) {
            float dist_to_end = distance(current_pos, end);
            if (dist_to_end < step) {
                cached_path.push_back(end);
                break;
            }

            vec2 dir_to_target = normalize(end - current_pos);
            float current_h = get_height_at_local_pos(current_pos.x, current_pos.y);

            float best_dist_score = FLT_MAX;
            vec2 best_next_pos = current_pos;
            bool found_valid_step = false;

            // Sample in a fan shape
            for (int s = 0; s < SAMPLES; s++) {
                float t = (float)s / (SAMPLES - 1); 
                float angle = -MAX_ANGLE + t * (2 * MAX_ANGLE);
                
                float cs = cos(angle);
                float sn = sin(angle);
                vec2 sample_dir = vec2(dir_to_target.x * cs - dir_to_target.y * sn, 
                                       dir_to_target.x * sn + dir_to_target.y * cs);
                
                vec2 candidate_pos = current_pos + sample_dir * step;
                
                if (abs(candidate_pos.x) > 0.5f || abs(candidate_pos.y) > 0.5f) continue;

                float candidate_h = get_height_at_local_pos(candidate_pos.x, candidate_pos.y);
                float height_diff = candidate_h - current_h;
                float actual_slope = height_diff / step;

                // --- THE FIX ---
                // 1. Hard Filter: Is the slope within tolerance?
                if (abs(actual_slope - target_slope) > SLOPE_TOLERANCE) continue;

                // 2. Tie-Breaker: Pick the valid candidate that moves closest to the destination
                float dist_score = distance(candidate_pos, end);

                if (dist_score < best_dist_score) {
                    best_dist_score = dist_score;
                    best_next_pos = candidate_pos;
                    found_valid_step = true;
                }
            }
            
            // If no valid step found (e.g., hit a cliff where even 85-degree turns 
            // result in a slope too steep), STOP drawing.
            if (!found_valid_step) break; 
            
            current_pos = best_next_pos;
            cached_path.push_back(current_pos);
        }

        return cached_path;
    }
    
    // ==========================================================
    // MODE 2: AUTO SLOPE (Greedy Best-First)
    // Tries to find the path of least resistance (minimizing steepness)
    // ==========================================================
    vector<vec2> generate_auto_slope_path(vec2 start, vec2 end, float max_allowed_slope, float step) {
        
        // Cache Check (reuse struct with mode 1)
        if (!cached_path.empty() && 
            distance(start, cached_path_data.start) < 0.001f && 
            distance(end, cached_path_data.end) < 0.05f && 
            cached_path_data.mode == 1 ) { 
            return cached_path;
        }

        cached_path_data = { start, end, max_allowed_slope, step, 1 };
        cached_path.clear();
        cached_path.push_back(start);

        vec2 current_pos = start;
        const int MAX_STEPS = 2000;
        
        const int SAMPLES = 7; 
        const float MAX_ANGLE = radians(60.0f);

        for(int i = 0; i < MAX_STEPS; i++) {
            float dist_to_end = distance(current_pos, end);
            if (dist_to_end < step) {
                cached_path.push_back(end);
                break;
            }

            vec2 dir_to_target = normalize(end - current_pos);
            float current_h = get_height_at_local_pos(current_pos.x, current_pos.y);

            float best_score = FLT_MAX;
            vec2 best_next_pos = current_pos + dir_to_target * step;

            for (int s = 0; s < SAMPLES; s++) {
                float t = (float)s / (SAMPLES - 1);
                float angle = -MAX_ANGLE + t * (2 * MAX_ANGLE);
                
                float cs = cos(angle);
                float sn = sin(angle);
                vec2 sample_dir = vec2(dir_to_target.x * cs - dir_to_target.y * sn, 
                                       dir_to_target.x * sn + dir_to_target.y * cs);
                
                vec2 candidate_pos = current_pos + sample_dir * step;
                
                if (abs(candidate_pos.x) > 0.5f || abs(candidate_pos.y) > 0.5f) continue;

                float candidate_h = get_height_at_local_pos(candidate_pos.x, candidate_pos.y);
                float height_diff = candidate_h - current_h;
                float actual_slope = abs(height_diff / step); // Absolute slope

                // SCORE CALCULATION:
                // 1. Steepness cost
                float steepness_cost = actual_slope * 2.0f;
                
                // 2. Distance cost (A* Heuristic: closer to goal is better)
                float dist_cost = distance(candidate_pos, end);

                // 3. Penalty if slope exceeds max allowed
                float penalty = 0.0f;
                if (actual_slope > max_allowed_slope) penalty = 1000.0f;

                float score = steepness_cost + dist_cost + penalty;

                if (score < best_score) {
                    best_score = score;
                    best_next_pos = candidate_pos;
                }
            }
            
            current_pos = best_next_pos;
            cached_path.push_back(current_pos);
        }

        return cached_path;
    }

private:
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
    float get_height_bilinear(vec2 p) { return get_height_bilinear(p.x,p.y); }

    float get_raw_height(int x, int y) {
        if (x < 0 || x >= hmap_width || y < 0 || y >= hmap_height) return 0.0f;
        size_t index = (size_t)y * hmap_width + x;

        if (is_16bit_data) {
            unsigned short* data_16bit = static_cast<unsigned short*>(height_data);
            unsigned short raw_value = data_16bit[index];
            return (float)raw_value / 65535.0f;
        } else {
            unsigned char* data_8bit = static_cast<unsigned char*>(height_data);
            unsigned char raw_value = data_8bit[index];
            return (float)raw_value / 255.0f;
        }
    }   
};

#endif

// #ifndef ELEVATIONLINEDRAWER_H
// #define ELEVATIONLINEDRAWER_H

// #include "textures/Texture.h"
// #include "settings/Settings.h"
// #include "Heightmap.h" 
// #include "world_objects/Line.h"
// #include <vector>
// #include <cmath>
// #include <algorithm>
// #include <glm/glm.hpp>
// #include <cfloat>

// // Helper for high-precision math
// #define PI 3.14159265359f
// using namespace glm;
// using namespace std;

// struct CachedPathData {
//     vec2 start, end;
//     float slope, step;
//     int mode; // 0 - match slope, 1 - auto
// };

// class ElevationLineDrawer
// {
// private:
//     float heightmap_scale;
//     void* height_data; 
//     std::vector<vec2> gradient_data;
//     bool is_16bit_data;
//     int hmap_width, hmap_height;
//     bool height_data_loaded = false;

//     vector<vec2> cached_path;
//     CachedPathData cached_path_data;

// public:
//     ElevationLineDrawer(const char* heightmap_path, float heightmap_scale, bool use_16bit = false) 
//         : heightmap_scale(heightmap_scale), is_16bit_data(use_16bit) 
//     {        
//         stbi_set_flip_vertically_on_load(true); 
//         int width, height, nrChannels;
//         if (is_16bit_data) {
//             height_data = stbi_load_16(heightmap_path, &width, &height, &nrChannels, 1);
//         } else {
//             height_data = stbi_load(heightmap_path, &width, &height, &nrChannels, 1);
//         }
//         hmap_width = width; hmap_height = height;

//         if (!height_data) {
//             std::cerr << "ERROR: Failed to load heightmap data." << std::endl;
//             return;
//         } else {
//             height_data_loaded = true;
//         }

//         /* load gradient data */
//         load_gradient();
//     }

//     ~ElevationLineDrawer() {
//         if (height_data_loaded && height_data) {
//             stbi_image_free(height_data);
//             gradient_data.clear();
//         }
//     }

//     /* uv is [0,1] terrain position, these function return local position and height  */
//     float get_height_at_uv(float u, float v) {
//         return get_height_bilinear(u*hmap_width,v*hmap_height) * heightmap_scale;
//     }
//     glm::vec3 get_local_pos_from_uv(float u, float v) {
//         u = glm::clamp(u,0.f,1.f); v = glm::clamp(v,0.f,1.f);
//         return vec3(u-.5f,v-.5f,get_height_at_uv(u,v));
//     }

//     /* local position is [-.5,.5] terrain position, these function return local position and height  */
//     float get_height_at_local_pos(float x, float y) {
//         if (x<-0.5f || y<-0.5f || x>0.5f || y>0.5f) return 0.f;
//         return get_height_bilinear((x+0.5f)*hmap_width,(y+0.5f)*hmap_height) * heightmap_scale;
//     }
//     glm::vec2 local_to_uv(glm::vec2 local) { return glm::vec2(local.x-0.5f,local.y-0.5f); }

//     /* Line drawing algorithm */
//     void clear_cache() { cached_path.clear(); }
    
//     vector<vec2> generate_constant_slope_path(vec2 start, vec2 end, float slope, float step, bool direction = true) {
        
//         if (!cached_path.empty() && 
//             distance(start, cached_path_data.start) < 0.001f && 
//             distance(end, cached_path_data.end) < 0.05f && 
//             abs(slope - cached_path_data.slope) < 0.001f &&
//             abs(step - cached_path_data.step) < 0.001f  &&
//             cached_path_data.mode == 0 ) { 
//             return cached_path;
//         }
        
//         cached_path_data = { start, end, slope, step, 0 };
//         cached_path.clear();
//         cached_path.push_back(start);

//         int max_safety_steps = 2000;
//         float min_dist_to_end = length(end - start);
//         int min_dist_index = 0;

//         for(int i = 0; i < max_safety_steps; i++) {
//             /* get target direction */
//             vec2 end_dir = (end-cached_path.back());
//             vec2 end_dir_normalise = end_dir / length(end_dir);
            
//             /* add new point */
//             vec2 points_uv = local_to_uv(cached_path.back());
//             float target_height = get_height_bilinear(points_uv.x, points_uv.y) + step*slope;
//             vec2 next_point = follow_slope_gradient_to_target_height(cached_path.back() + end_dir_normalise*step, target_height);
//             cached_path.push_back( next_point );

//             /* calculate final distances */
//             float dist = length(end-cached_path.back());
//             float points_dist = length(cached_path.back()-cached_path[cached_path.size()-2]);
//             if (dist < min_dist_to_end) {
//                 min_dist_to_end = dist;
//                 min_dist_index = cached_path.size()-1;
//             }

//             /* exit conditions */
//             if (points_dist < 0.5*step) break;
//             if (dist > min_dist_to_end + step*20.f) break; // heuristic
//             if (dist < step) break;
//         }

//         /* Path smoothing */
//         if (cached_path.size() > 2) {
//             vector<vec2> smoothed = cached_path;
//             for(int i = 1; i < cached_path.size() - 1; i++) {
//                 smoothed[i] = (cached_path[i-1] + cached_path[i] + cached_path[i+1]) / 3.0f;
//             }
//             cached_path = smoothed;
//         }

//         if (min_dist_index < cached_path.size() - 1) cached_path.resize(min_dist_index+1);
//         return cached_path;
//     }
    
//     vector<vec2> generate_auto_slope_path(vec2 start, vec2 end, float max_slope, float step) {
//         /* first generate a straight path with equally spaced points*/
//         float path_dist = glm::length(end-start);
//         int point_num = (int)(path_dist / step) + 2;
//         std::vector<vec2> points; points.resize(point_num);
//         points[0] = start;
//         for (int i=1; i<point_num-1; i++) {
//             float t = (float)i / (point_num-1);
//             points[i] = vec2(end.x*t + start.x*(1.f-t), end.y*t + start.y*(1.f-t));
//         }
//         points[point_num-1] = end;

//         /* path data */
//         float start_height = get_height_bilinear(start);
//         float delta_height = get_height_bilinear(end) - start_height;

//         /* for each point in straight path check if height above / below ideal constatnt slope path */
//         /* then follow gradient to that target */
//         const int mini_steps_num = 16;
//         for (int mini_step=0; mini_step<mini_steps_num; mini_step++){
//             for (int i=1; i<point_num-1; i++) {
//                 float target_height = start_height + ((float)i / (point_num-1)) * delta_height;
//                 float current_height = get_height_bilinear(points[i]);
//                 float diff = glm::abs(target_height-current_height) *1000000.f ;
//                 bool dir = current_height < target_height;
//                 points[i] = follow_slope_gradient_set_distance(points[i], diff, dir);
//             }
//         }

//         return points;
//     }

// private:
//     // this function returns the height value for any x,y given in fractional pixel values
//     // eg. pixel value of x=1.5f is average height from pixel 1 and pixel 2 together
//     float get_height_bilinear(float x, float y) {
//         if (!height_data_loaded) return 0.f;
//         int x0 = (int)x; int y0 = (int)y;
//         int x1 = std::min(x0 + 1, hmap_width - 1);
//         int y1 = std::min(y0 + 1, hmap_height - 1);
//         float sx = x - (float)x0;
//         float sy = y - (float)y0;
//         float h00 = get_raw_height(x0, y0);
//         float h10 = get_raw_height(x1, y0);
//         float h01 = get_raw_height(x0, y1);
//         float h11 = get_raw_height(x1, y1);
//         float h0 = glm::mix(h00, h10, sx);
//         float h1 = glm::mix(h01, h11, sx);
//         return glm::mix(h0, h1, sy);
//     }
//     float get_height_bilinear(vec2 p) { return get_height_bilinear(p.x,p.y); }

//     float get_raw_height(int x, int y) {
//         if (x < 0 || x >= hmap_width || y < 0 || y >= hmap_height) return 0.0f;
//         size_t index = (size_t)y * hmap_width + x;

//         if (is_16bit_data) {
//             // Rzutowanie void* na unsigned short* i odczyt 16-bitowej wartości
//             unsigned short* data_16bit = static_cast<unsigned short*>(height_data);
//             unsigned short raw_value = data_16bit[index];
//             // Normalizacja 16-bitowej wartości (0-65535) do float (0.0-1.0)
//             return (float)raw_value / 65535.0f; 
//         } else {
//             // Rzutowanie void* na unsigned char* i odczyt 8-bitowej wartości
//             unsigned char* data_8bit = static_cast<unsigned char*>(height_data);
//             unsigned char raw_value = data_8bit[index];
//             // Normalizacja 8-bitowej wartości (0-255) do float (0.0-1.0)
//             return (float)raw_value / 255.0f;
//         }
//     }   

//     vec2 follow_slope_gradient_to_target_height(vec2 pos, float target_height) {
//         const int MAX_ITERATIONS = 8; // Reduced iterations for performance
//         const float DISTANCE_EPSILON = 0.001f; 
//         float eps = 1.0f / (float)hmap_width; 

//         for (int i = 0; i < MAX_ITERATIONS; i++) {
//             float current_h = get_height_at_local_pos(pos.x, pos.y);
//             float diff = target_height - current_h;

//             if (abs(diff) < DISTANCE_EPSILON) return pos;

//             vec2 gradient = get_gradient_bilinear(pos);
//             float grad_len_sq = dot(gradient, gradient); // huh?

//             if (grad_len_sq < 0.000001f) break; // Flat terrain

//             // Limit step size to avoid shooting off into infinity on flat slopes
//             vec2 offset = gradient * (diff / grad_len_sq);
//             float offset_len = length(offset);
//             if(offset_len > 0.05f) offset = (offset / offset_len) * 0.05f; // Cap jump size

//             pos += offset;
//             pos = clamp(pos, -0.5f, 0.5f);
//         }
//         return pos;
//     }
    
//     vec2 follow_slope_gradient_set_distance(vec2 origin, float distance, bool upwards) {
//         const int mini_step_number = 2;
//         float mini_step_distance = distance / mini_step_number;
//         vec2 current_pos = origin;
//         float direction = upwards ? 1.f : (-1.f);
    
//         for (int i=0; i<mini_step_number; i++) {
//             vec2 local_gradient = direction * get_gradient_bilinear(current_pos);
//             current_pos += glm::normalize(local_gradient) * mini_step_distance;
//         }
    
//         return glm::clamp(current_pos,-0.5f,0.5f);
//     }

//     vec2 get_gradient_bilinear(float x, float y) {
//         if (!height_data_loaded) return vec2(0);
//         int x0 = (int)x; int y0 = (int)y;
//         int x1 = std::min(x0 + 1, hmap_width - 1);
//         int y1 = std::min(y0 + 1, hmap_height - 1);
//         float sx = x - (float)x0;
//         float sy = y - (float)y0;
//         vec2 h00 = get_raw_gradient(x0, y0);
//         vec2 h10 = get_raw_gradient(x1, y0);
//         vec2 h01 = get_raw_gradient(x0, y1);
//         vec2 h11 = get_raw_gradient(x1, y1);
//         vec2 h0 = vec2(glm::mix(h00.x, h10.x, sx),glm::mix(h00.y, h10.y, sx));
//         vec2 h1 = vec2(glm::mix(h01.x, h11.x, sx),glm::mix(h01.y, h11.y, sx));
//         return vec2(glm::mix(h0.x, h1.x, sy),glm::mix(h0.y, h1.y, sy));
//     }
//     vec2 get_gradient_bilinear(vec2 g) { return get_gradient_bilinear(g.x, g.y); }

//     vec2 get_raw_gradient(int x, int y) {
//         if (x < 0 || x >= hmap_width || y < 0 || y >= hmap_height) return vec2(0);
//         int i = y * hmap_width + x;
//         return gradient_data[i];
//     }

//     void load_gradient() {

//         gradient_data.clear();
//         gradient_data.resize(hmap_width*hmap_height);

//         for (int x=0; x<hmap_width; x++){
//             for (int y=0; y<hmap_width; y++){
//                 const int eps = 1;
//                 float h_x1 = get_raw_height(x + eps, y);
//                 float h_x2 = get_raw_height(x - eps, y);
//                 float grad_x = (h_x1 - h_x2) / (2.0f * eps);

//                 float h_y1 = get_raw_height(x, y + eps);
//                 float h_y2 = get_raw_height(x, y - eps);
//                 float grad_y = (h_y1 - h_y2) / (2.0f * eps);

//                 int idx = y*hmap_width + x;
//                 gradient_data[idx] = vec2(grad_x, grad_y);
//             }
//         }
//     }
// };

// #endif