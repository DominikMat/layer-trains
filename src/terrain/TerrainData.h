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
    float snow_max_steepness = 0.065f;  // How flat ground must be for snow at the lowest level
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
    /* snow_level_height: */ 1800.f
};

enum BlueRegions {
    INTERACTABLE = 255, 
    NATURE_RESERVE = 239,
    CITY = 223,
    BLUE_RESERVED0 = 207,
    BLUE_RESERVED1 = 191,
    BLUE_RESERVED2 = 175,
    BLUE_RESERVED3 = 159,
    BLUE_RESERVED4 = 127,
    BLUE_RESERVED5 = 111,
    BLUE_RESERVED6 = 95,
    BLUE_RESERVED7 = 79,
    BLUE_RESERVED8 = 63,
    BLUE_RESERVED9 = 47,
    BLUE_RESERVED10 = 31,
    BLUE_RESERVED11 = 15,
    BLUE_NONE = 0
};
enum GreenRegions {
    NAME_TAG = 255, 
    FORREST = 239,
    SAND = 223,
    WATER = 207,
    GREEN_RESERVED1 = 191,
    GREEN_RESERVED2 = 175,
    GREEN_RESERVED3 = 159,
    GREEN_RESERVED4 = 127,
    GREEN_RESERVED5 = 111,
    GREEN_RESERVED6 = 95,
    GREEN_RESERVED7 = 79,
    GREEN_RESERVED8 = 63,
    GREEN_RESERVED9 = 47,
    GREEN_RESERVED10 = 31,
    GREEN_RESERVED11 = 15,
    GREEN_NONE = 0,
};

static const std::unordered_map<int, vec4> BLUE_REGION_COLOURS = {
    { BlueRegions::INTERACTABLE, Colour::TRANSPARENT }, // Interactable (Invisible on map, logic handled separately)
    { BlueRegions::NATURE_RESERVE, Colour::GREEN }, // Nature Reserve (Dark Green tint)
    { BlueRegions::CITY, Colour::YELLOW }, // City Area (Grey-Blue tint)
    { BlueRegions::BLUE_RESERVED0,   Colour::TRANSPARENT } , // Default (No tint)
    { BlueRegions::BLUE_RESERVED1,   Colour::TRANSPARENT } , // Default (No tint)
    { BlueRegions::BLUE_RESERVED2,   Colour::TRANSPARENT } , // Default (No tint)
    { BlueRegions::BLUE_RESERVED3,   Colour::TRANSPARENT } , // Default (No tint)
    { BlueRegions::BLUE_RESERVED4,   Colour::TRANSPARENT } , // Default (No tint)
    { BlueRegions::BLUE_RESERVED5,   Colour::TRANSPARENT } , // Default (No tint)
    { BlueRegions::BLUE_RESERVED6,   Colour::TRANSPARENT } , // Default (No tint)
    { BlueRegions::BLUE_RESERVED7,   Colour::TRANSPARENT } , // Default (No tint)
    { BlueRegions::BLUE_RESERVED8,   Colour::TRANSPARENT } , // Default (No tint)
    { BlueRegions::BLUE_RESERVED9,   Colour::TRANSPARENT } , // Default (No tint)
    { BlueRegions::BLUE_RESERVED10,   Colour::TRANSPARENT },  // Default (No tint)
    { BlueRegions::BLUE_RESERVED11,   Colour::TRANSPARENT },  // Default (No tint)
    { BlueRegions::BLUE_NONE,   Colour::TRANSPARENT }  // Default (No tint)
};
static const std::unordered_map<int, vec4> GREEN_REGION_COLOURS = {
    { GreenRegions::NAME_TAG, Colour::TRANSPARENT }, // Forest
    { GreenRegions::FORREST, vec4(0.76f, 0.70f, 0.50f, 1.0f) }, // Sand
    { GreenRegions::SAND, vec4(0.00f, 0.30f, 0.70f, 1.0f) }, // Water
    { GreenRegions::WATER,   Colour::SKY_BLUE },  // Default grass/ground
    { GreenRegions::GREEN_RESERVED1,   Colour::TRANSPARENT } , // Default (No tint)
    { GreenRegions::GREEN_RESERVED2,   Colour::TRANSPARENT } , // Default (No tint)
    { GreenRegions::GREEN_RESERVED3,   Colour::TRANSPARENT } , // Default (No tint)
    { GreenRegions::GREEN_RESERVED4,   Colour::TRANSPARENT } , // Default (No tint)
    { GreenRegions::GREEN_RESERVED5,   Colour::TRANSPARENT } , // Default (No tint)
    { GreenRegions::GREEN_RESERVED6,   Colour::TRANSPARENT } , // Default (No tint)
    { GreenRegions::GREEN_RESERVED7,   Colour::TRANSPARENT } , // Default (No tint)
    { GreenRegions::GREEN_RESERVED8,   Colour::TRANSPARENT } , // Default (No tint)
    { GreenRegions::GREEN_RESERVED9,   Colour::TRANSPARENT } , // Default (No tint)
    { GreenRegions::GREEN_RESERVED10,   Colour::TRANSPARENT },  // Default (No tint)
    { GreenRegions::GREEN_RESERVED11,   Colour::TRANSPARENT },  // Default (No tint)
    { GreenRegions::GREEN_NONE,   Colour::TRANSPARENT }  // Default grass/ground
};


// 3. BORDER SETTINGS

#define BLUE_REGION_OPACITY 0.15f
#define GREEN_REGION_OPACITY 0.15f

const vec4 BORDER_COLOUR = vec4(0.f,0.f,0.f,BLUE_REGION_OPACITY*3.f);

#endif