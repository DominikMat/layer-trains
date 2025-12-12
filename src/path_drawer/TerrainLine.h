#ifndef TERRAINLINE_H
#define TERRAINLINE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "Line.h"
#include "Texture.h"
#include "TerrainData.h"

class TerrainLine : public Line
{
    const TerrainData *td;
public:
    TerrainLine(const TerrainData *td) : Line(PATH_THICKNESS), td(td) {
    }
    
    void initialize_shader_properties() override {
        Shader *shader = new TERRAIN_LINE_SHADER;
        shader->use();

        shader->setVec3("line_colour", PATH_COLOUR );
        shader->setVec3("max_steepness_colour", Colour::RED );
        shader->setVec3("min_steepness_colour", Colour::BLUE );
        shader->setBool("show_steepness", true);
        
        shader->addTexture(new Texture(td->heightmap_path,true,true)); shader->setInt("heightmap", shader->get_last_loaded_tex_slot());
        shader->setFloat("heightmap_scale", td->vertical_scale);
        shader->setFloat("steepness_scale", STEEPNESS_SCALE);
        shader->setInt("heightmap_resolution_x", td->resolution_x);
        shader->setInt("heightmap_resolution_y", td->resolution_y);

        shader->setFloat("terrain_offset_distance", PATH_TERRAIN_OFFSET_DIST);
        //shader->setFloat("line_thickness", PATH_THICKNESS);

        shader->setFloat("max_steepness_value", 1.f);

        set_shader(shader);
    }
};

#endif 