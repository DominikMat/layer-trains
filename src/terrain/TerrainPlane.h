#ifndef TERRAINPLANE_H
#define TERRAINPLANE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <regex>
#include "world_objects/Object.h"
#include "world_objects/Plane.h"
#include "rendering/Camera.h"
#include "Text.h"
#include "settings/Settings.h"
#include "ElevationLineDrawer.h"
#include "InteractableManager.h"
#include "TerrainPainter.h"
#include "TerrainData.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;

class TerrainPlane : public Plane
{
    bool shaderInitalized = false;
    const TerrainData *td;

public:
    Camera *cam;
    TerrainPlane(const TerrainData *td, Camera *cam, vec3 pos = vec3(0.0f,0.0f,0.0f), vec3 size = vec3(1.0f,1.0f,1.0f) )
        : Plane(glm::max(td->resolution_x,td->resolution_y),pos,size), td(td), cam(cam)
    {}

    void configure_render_properties() override {
        //shader->setFloat("camera_zoom_level", cam->get_current_orthographic_zoom());

        if (shaderInitalized) return;
        else shaderInitalized = true;

        // --- Terrain height data ---
        shader->setFloat("terrain_min_height", td->minimum_height_reach);
        shader->setFloat("terrain_max_height", td->maximum_height_reach);
        shader->setFloat("heightmap_resolution_x", td->resolution_x);
        shader->setFloat("heightmap_resolution_y", td->resolution_y);
        shader->setFloat("heightmap_scale", td->vertical_scale);
        shader->setBool("heightmap_enabled", true);
        shader->setVec3("terrain_boundary_colour", Colour::TERRAIN_SIDE_COLOUR);
        shader->setInt("terrain_boundrary_pixel_width", TERRAIN_BOUNDARY_PIXEL_NUM);
        //shader->addTexture(new Texture(td->heightmap_path)); shader->setInt("heightmap",shader->get_last_loaded_tex_slot());
        shader->addTexture(new Texture(td->areas_data_path)); shader->setInt("terrain_area_data",shader->get_last_loaded_tex_slot());

        // --- Terrain contour lines ---
        shader->setFloat("iso_line_spacing", ISO_LINE_SPACING);
        shader->setFloat("iso_line_thickness", ISO_LINE_THICKNESS);
        shader->setVec4("iso_line_colour", CONTOUR_LINE_COLOUR);

        // --- Terrain colour pallete ---
        shader->addTexture(new GRADIENT_ELEVATION); shader->setInt("elevation_gradient",shader->get_last_loaded_tex_slot());
        shader->addTexture(new GRADIENT_STEEPNESS); shader->setInt("steepness_gradient", shader->get_last_loaded_tex_slot());
        shader->addTexture(new GRADIENT_WATER); shader->setInt("water_gradient", shader->get_last_loaded_tex_slot());
        shader->setFloat("elevation_gradient_max_height", ELEVATION_GRADIENT_MAX_HEIGHT);
        shader->setFloat("elevation_gradient_strength", ELEVATION_GRADIENT_STRENGTH);
        shader->setFloat("steepness_scale", STEEPNESS_SCALE);
        
        // --- Terrain water parameters ---
        shader->setVec3("water_inside_colour", Colour::SKY_BLUE);
        shader->setVec3("water_outline_colour", Colour::BLUE);
        shader->setFloat("water_level_height", td->water_level_height);
        
        // --- Terrain snow parameters ---
        shader->setFloat("snow_level_height", td->snow_level_height);
        shader->setFloat("snow_falloff_range", SNOW_FALLOFF_RANGE);
        shader->setFloat("snow_max_steepness", SNOW_MAX_STEEPNESS);
        shader->setVec4("snow_colour", Colour::SNOW_COLOUR);
    }
};

#endif