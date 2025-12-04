#ifndef TERRAINDATA_H
#define TERRAINDATA_H

struct TerrainData
{
    const char* title;
    const char* heightmap_path;
    const char* areas_data_path;

    int resolution_x;
    int resolution_y;

    float minimum_height_reach;
    float maximum_height_reach;
    float vertical_scale;
    float water_level_height = 0.f;
    float snow_level_height = 3000.f;
    float snow_falloff_range = 1000.f; // Distance above snow level where snow covers everything
    float snow_max_steepness = 0.35f;  // How flat ground must be for snow at the lowest level
};

const TerrainData terrain_transalpine = {
    /* title: */ "Transalpine pass, Romania",
    /* heightmap_path: */ "C:/Media/Projects/OpenGL/Layer_Trains/textures/terrain/transalpine_heightmap.png",
    /* areas_data_path: */ "C:/Media/Projects/OpenGL/Layer_Trains/textures/terrain/transalpine_area_data.png",
    /* resolution_x: */ 1024,
    /* resolution_y: */ 1024,
    /* minimum_height_reach: */ 1000.f,
    /* maximum_height_reach: */ 2500.f,
    /* vertical_scale: */ 295.07f / 1024.f,
    /* water_level_height: */ 0.f,
    /* snow_level_height: */ 2000.f
};

#endif