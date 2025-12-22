#ifndef LEVEL_MANAGER_H
#define LEVEL_MANAGER_H

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "TerrainData.h"
#include "json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

class LevelManager {
private:
    const std::string LEVELS_PATH = "C:/Media/Projects/OpenGL/Layer_Trains/user-created/levels/";
    std::vector<TerrainData> loaded_levels;

public:
    LevelManager() {
        if (!fs::exists(LEVELS_PATH)) fs::create_directories(LEVELS_PATH);
    }

    void load_user_levels() {
        loaded_levels.clear();
        for (const auto& entry : fs::directory_iterator(LEVELS_PATH)) {
            if (entry.path().extension() == ".json") {
                load_single_level(entry.path().string());
            }
        }
        std::cout << "LevelManager: Loaded " << loaded_levels.size() << " user levels." << std::endl;
    }

    // Returns a vector of structs ready to be used by Scenes
    std::vector<TerrainData>& get_level_data() {
        return loaded_levels; // Return by reference to avoid copying whole vectors
    }

private:
    void load_single_level(const std::string& path) {
        std::ifstream i(path);
        if (!i.good()) return;
        json j;
        i >> j;

        TerrainData data;
        data.title = j.value("title", "Untitled");
        data.heightmap_path = j.value("heightmap_path", "");
        data.areas_data_path = j.value("areas_data_path", "");

        // 2. Load Basic Data
        data.resolution_x = j.value("resolution_x", 1024);
        data.resolution_y = j.value("resolution_y", 1024);
        data.minimum_height_reach = j.value("min_height", 0.f);
        data.maximum_height_reach = j.value("max_height", 1000.f);
        data.vertical_scale = j.value("vertical_scale", 1.f);
        data.water_level_height = j.value("water_level", 0.f);
        data.snow_level_height = j.value("snow_level", 3000.f);

        // 3. Load Tags
        // Initialize default empty tags
        for(int k=0; k<MAX_TAG_AMOUNT; k++) {
            data.tags[k].type = TerrainTagType::DISABLED;
            data.tags[k].name = ""; 
        }

        if (j.contains("tags")) {
            int idx = 0;
            for (auto& element : j["tags"]) {
                if (idx >= MAX_TAG_AMOUNT) break;
                data.tags[idx].uv_x = element["uv_x"];
                data.tags[idx].uv_y = element["uv_y"];
                data.tags[idx].name = element.value("name", "Tag");
                data.tags[idx].type = (TerrainTagType)element["type"];
                idx++;
            }
        }
        loaded_levels.push_back(data);
    }
};

#endif