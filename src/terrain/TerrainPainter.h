#ifndef TERRAINPAINTER_H
#define TERRAINPAINTER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <regex>
#include "world_objects/Object.h"
#include "world_objects/Plane.h"
#include "rendering/Camera.h"
#include "settings/Settings.h"
#include "ElevationLineDrawer.h"
#include "TerrainData.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define ERROR_EMPTY_TEXTURE_RETURN Texture(1, 1, new unsigned char[3]{0,0,0})

#define TERRAIN_BOUNDARY_PIXEL_NUM 2

using namespace glm;
using namespace std;

class TerrainPainter
{
public:
    //bool data_loaded = false;
    //vector<float> height_data_raw;
    vector<vec2> terrain_areas_data;
    vector<vec2> interactable_positions;
    vector<vec2> name_tag_positions;
    const TerrainData *terrain_data;

    TerrainPainter (const TerrainData *terrain_data) {
        this->terrain_data = terrain_data;
    }

    Texture bake_terrain_texture(bool release_data = true) {
        #if OVERWRITE_CACHED_TEXTURES
            bool debug_overwrite = true;
        #else
            bool debug_overwrite = false;
        #endif

        // chceck if a generated texture already present - if so and no flag, use it
        std::string cleaned_name = clean_map_name(terrain_data->title);
        std::string cache_path = std::string(TEXTURE_GENERATED_CACHE_FOLDER_PATH) + "/" + cleaned_name + "_colour_texture.png";

        if (!debug_overwrite && std::ifstream(cache_path).good()) {
            return Texture(cache_path.c_str(), false);
        }

        // load terrain data
        int width, height, nrChannels;
        unsigned char* map_data = stbi_load(terrain_data->areas_data_path, &width, &height, &nrChannels, 3);
        if (!map_data) return ERROR_EMPTY_TEXTURE_RETURN;


        // load painting gradients
        int grad_w;
        std::vector<vec3> grad_elev = load_gradient_data(GRADIENT_ELEVATION_PATH, grad_w);
        std::vector<vec3> grad_steep = load_gradient_data(GRADIENT_STEEPNESS_PATH, grad_w);
        std::vector<vec3> grad_water = load_gradient_data(GRADIENT_WATER_PATH, grad_w);

        // init output colour buffer and arrays
        std::vector<unsigned char> color_buffer(width * height * 3);
        interactable_positions.clear();
        name_tag_positions.clear();

        // helper function to query data at pixel
        auto get_pixel = [&](int x, int y) -> glm::ivec3 {
            int cx = glm::clamp(x, 0, width - 1);
            int cy = glm::clamp(y, 0, height - 1);
            int idx = (cy * width + cx) * 3;
            return glm::ivec3(map_data[idx], map_data[idx+1], map_data[idx+2]);
        };

        // Get Normalized Height (0.0 - 1.0) from Red Channel
        auto get_h_norm = [&](int x, int y) -> float {
            return (float)get_pixel(x, y).r / 255.0f;
        };

        // calculate colour for every pixel of output terrain
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = (y * width + x) * 3;

                /* ================================================ */
                /* COLOUR TERRAIN BOUNDARY  */

                if (y < TERRAIN_BOUNDARY_PIXEL_NUM || x < TERRAIN_BOUNDARY_PIXEL_NUM || x >= width-TERRAIN_BOUNDARY_PIXEL_NUM || y >= height-TERRAIN_BOUNDARY_PIXEL_NUM) {
                    color_buffer[idx] = (unsigned char)(Colour::TERRAIN_SIDE_COLOUR.r * 255.f);
                    color_buffer[idx+1] = (unsigned char)(Colour::TERRAIN_SIDE_COLOUR.g * 255.f);
                    color_buffer[idx+2] = (unsigned char)(Colour::TERRAIN_SIDE_COLOUR.b * 255.f);
                    continue;
                }

                /* ================================================ */
                /* ELEVATION AND STEEPNESS COLOURING  */

                // get height at pixel
                float get_height_at_pixel = get_pixel(x,y).r;
                float norm_h = get_height_at_pixel / 255.f;
                float pixel_elevation = terrain_data->minimum_height_reach + norm_h * (terrain_data->maximum_height_reach - terrain_data->minimum_height_reach);
                vec3 final_colour;

                // get steepness at pixel
                const int S = STEEPNESS_SMOOTHING_STEP_SIZE;
                float dx = (get_h_norm(x+S,y) - get_h_norm(x-S,y)) / (2.f*S);
                float dy = (get_h_norm(x,y+S) - get_h_norm(x,y-S)) / (2.f*S);
                float steepness = glm::length(vec2(dx, dy));

                // find elevation region
                bool is_under_water_level = pixel_elevation <= terrain_data->water_level_height;
                bool is_dry_land = !is_under_water_level;
                bool is_above_snow_level = pixel_elevation >= terrain_data->snow_level_height;

                // apply water gradient
                if (is_under_water_level) {
                    float depth = glm::clamp((terrain_data->water_level_height- pixel_elevation) / terrain_data->water_level_height, 0.f, 1.f);
                    final_colour = sample_gradient(grad_water, depth * depth);
                }

                // apply dry land - elevation and steepness gradients
                if (is_dry_land) {
                    float t_elev = glm::clamp(pixel_elevation / ELEVATION_GRADIENT_MAX_HEIGHT, 0.f, 1.f);
                    float t_steep = glm::clamp(steepness * STEEPNESS_SCALE, 0.f, 1.f);
                    final_colour = glm::mix(sample_gradient(grad_steep, t_steep), sample_gradient(grad_elev, t_elev), ELEVATION_GRADIENT_STRENGTH);
                }

                // apply extra snow layer
                if (is_above_snow_level) {

                    float height_above = pixel_elevation - terrain_data->snow_level_height;
                    float above_snow_level_mult = height_above / SNOW_FALLOFF_RANGE;
                    float falloff_ratio = glm::clamp(above_snow_level_mult*above_snow_level_mult, 0.0f, 1.0f);
                    float allowed_steepness = glm::mix(SNOW_MAX_STEEPNESS, 1.0f, falloff_ratio);
                    
                    if (steepness * STEEPNESS_SCALE < allowed_steepness) {
                        float snow_transition = glm::clamp(height_above / 50.0f, 0.0f, 1.0f); // Smooth transition at the very bottom edge of snow line
                        float snow_opacity = snow_transition; //glm::clamp(snow_transition, 0.f, 1.f);
                        final_colour = glm::mix(final_colour, vec3(Colour::SNOW_COLOUR), snow_opacity*Colour::SNOW_COLOUR.a);
                    }
                }
                
                /* ================================================ */
                /* REGION SPECIFIC COLOURING  */
                
                // blue region check
                if (is_border_pixel(x,y,width,height,map_data)) final_colour = BORDER_COLOUR;
                else {
                    float pixel_blue_channel = get_pixel(x,y).b;
                    BlueRegions blue_region = ((int)(pixel_blue_channel+1) % 16) == 0 ? (BlueRegions)(int)(pixel_blue_channel) : BlueRegions::BLUE_NONE;
                    if (blue_region != BlueRegions::BLUE_NONE) {
                        vec4 blue_region_colour = get_color_from_map(BLUE_REGION_COLOURS, blue_region);
                        final_colour = glm::mix(final_colour, vec3(blue_region_colour), BLUE_REGION_OPACITY);
                    }
                    else if (is_border_pixel(x,y,width,height,map_data)) final_colour = BORDER_COLOUR;
                    
                    // green region check
                    float pixel_green_channel = get_pixel(x,y).g;
                    GreenRegions green_region = ((int)(pixel_green_channel+1) % 16) == 0 ? (GreenRegions)(int)(pixel_green_channel) : GreenRegions::GREEN_NONE;
                    if (green_region != GreenRegions::GREEN_NONE) {
                        vec4 green_region_colour = get_color_from_map(GREEN_REGION_COLOURS, green_region);
                        final_colour = glm::mix(final_colour, vec3(green_region_colour), GREEN_REGION_OPACITY);
                    }
                }
                
                                
                // apply final colour
                color_buffer[idx] = (unsigned char)(final_colour.r * 255.f);
                color_buffer[idx+1] = (unsigned char)(final_colour.g * 255.f);
                color_buffer[idx+2] = (unsigned char)(final_colour.b * 255.f);
            }
        }

        // release raw data
        if (release_data) {
            stbi_image_free(map_data);
        }

        // teturn generated png as texture
        stbi_write_png(cache_path.c_str(), width, height, 3, color_buffer.data(), width * 3);
        return Texture(width, height, color_buffer.data());
    }

    vector<vec2> get_interactable_positions() {
        return interactable_positions;
    }
    vector<vec2> get_name_tag_positions() {
        return name_tag_positions;
    }

private:
    static std::string clean_map_name(const std::string& name) {
        std::string cleaned = name;
        cleaned = std::regex_replace(cleaned, std::regex(", "), "-");
        std::replace(cleaned.begin(), cleaned.end(), ' ', '_');
        return cleaned;
    }

    static std::vector<vec3> load_gradient_data(const char* path, int& out_width) {
        int height, nrChannels;
        unsigned char *data = stbi_load(path, &out_width, &height, &nrChannels, 3);
        std::vector<vec3> gradient;
        if (data && out_width > 0) {
            for (int i = 0; i < out_width; ++i) {
                gradient.push_back(vec3(
                    (float)data[i * 3 + 0] / 255.0f,
                    (float)data[i * 3 + 1] / 255.0f,
                    (float)data[i * 3 + 2] / 255.0f
                ));
            }
        }
        stbi_image_free(data);
        return gradient;
    }

    static vec3 sample_gradient(const std::vector<vec3>& gradient, float t) {
        if (gradient.empty()) return vec3(0.f);
        t = glm::clamp(t, 0.0f, 1.0f);
        int index = (int)(t * (gradient.size() - 1));
        return gradient[index];
    }
     
    // Checks 4 neighbors. If Blue (Region) or Green (Biome) differs, it's a border.
    bool is_border_pixel(int x, int y, int w, int h, unsigned char* data) {
        if (x <= 0 || x >= w-1 || y <= 0 || y >= h-1) return false;

        int idx = (y * w + x) * 3;
        unsigned char my_g = data[idx+1];
        unsigned char my_b = data[idx+2];

        // Check neighbors (Up, Down, Left, Right)
        int offsets[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };

        for (auto& off : offsets) {
            int n_idx = ((y + off[1]) * w + (x + off[0])) * 3;
            // Compare Biome (Green) and Region (Blue)
            // Note: We ignore Height (Red) for borders
            if (data[n_idx+1] != my_g || data[n_idx+2] != my_b) {
                return true;
            }
        }
        return false;
    }

    // Helper to fetch color from map with default fallback
    vec4 get_color_from_map(const std::unordered_map<int, vec4>& map, int key) {
        auto it = map.find(key);
        if (it != map.end()) {
            return it->second;
        }
        // Check for "0" default key
        it = map.find(0);
        if (it != map.end()) return it->second;
        
        return Colour::MAGENTA; // Debug color for missing definitions
    }
};

#endif