

#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include "textures/Texture.h"
#include "settings/Settings.h"

class Heightmap
{
public:
    Texture height_data;
    float min_height, max_height, scale, water_level_height;
    const char* map_name;

    Heightmap(Texture height_data, float min_height, float max_height, float scale, const char* map_name = "Default Heightmap")
        : height_data(height_data), min_height(min_height), max_height(max_height), scale(scale), water_level_height(WATER_LEVEL_HEIGHT_DEFAULT), map_name(map_name)
    { }
};
#endif