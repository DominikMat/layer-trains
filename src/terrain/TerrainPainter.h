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

#define ERROR_EMPTY_TEXTURE_RETURN Texture(1, 1, new unsigned char[3]{0,0,0}, 1)

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
            return Texture(cache_path.c_str(), 0, false); 
        }
        
        // load raw height data
        int width, height, nrChannels;
        unsigned char* height_data_raw = stbi_load(terrain_data->heightmap_path, &width, &height, &nrChannels, 1);
        if (!height_data_raw) return ERROR_EMPTY_TEXTURE_RETURN; 
 
        // load raw additional terrain data (special sections, biomes)

        // load painting gradients
        int grad_w;
        std::vector<vec3> grad_elev = load_gradient_data(GRADIENT_ELEVATION_PATH, grad_w);
        std::vector<vec3> grad_steep = load_gradient_data(GRADIENT_STEEPNESS_PATH, grad_w);
        std::vector<vec3> grad_water = load_gradient_data(GRADIENT_WATER_PATH, grad_w);

        // init output colour buffer
        std::vector<unsigned char> color_buffer(width * height * 3);
        float height_range = terrain_data->maximum_height_reach - terrain_data->minimum_height_reach;
            
        // helper function to query data at pixel
        auto get_h = [&](int x, int y) -> float {
            return (float)height_data_raw[glm::clamp(y,0,height-1)*width + glm::clamp(x,0,width-1)] / 255.0f;
        };

        // calculate colour for every pixel of output terrain
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = (y * width + x) * 3;
                float norm_h = get_h(x, y);
                float curr_elev = terrain_data->minimum_height_reach + norm_h * height_range;
                vec3 finalColor;

                // water 
                if (curr_elev < terrain_data->water_level_height) {
                    float depth = glm::clamp((terrain_data->water_level_height- curr_elev) / terrain_data->water_level_height, 0.f, 1.f);
                    finalColor = sample_gradient(grad_water, depth * depth);
                }

                // dry land
                else {
                    const int S = STEEPNESS_SMOOTHING_STEP_SIZE;
                    float dx = (get_h(x+S,y) - get_h(x-S,y)) / (2.f*S);
                    float dy = (get_h(x,y+S) - get_h(x,y-S)) / (2.f*S);
                    float steepness = glm::length(vec2(dx, dy));
                    
                    float t_elev = glm::clamp(curr_elev / ELEVATION_GRADIENT_MAX_HEIGHT, 0.f, 1.f);
                    float t_steep = glm::clamp(steepness * STEEPNESS_SCALE, 0.f, 1.f);
                    finalColor = glm::mix(sample_gradient(grad_steep, t_steep), sample_gradient(grad_elev, t_elev), ELEVATION_GRADIENT_STRENGTH);
                }

                color_buffer[idx] = (unsigned char)(finalColor.r * 255.f);
                color_buffer[idx+1] = (unsigned char)(finalColor.g * 255.f);
                color_buffer[idx+2] = (unsigned char)(finalColor.b * 255.f);
            }
        }

        // release raw data
        if (release_data) {
            stbi_image_free(height_data_raw);
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
};

#endif