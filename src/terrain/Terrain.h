#ifndef TERRAIN_H
#define TERRAIN_H

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
#include "InteractableManager.h"
#include "TerrainPainter.h"
#include "TerrainData.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;

class Terrain
{
public:
    Plane terrain_obj;
    Shader terrain_shader;
    ElevationLineDrawer elevation_line_drawer;
    const TerrainData *terrain_data;
    Texture heightmap_texture;

    vector<Interactable*> attached_interactables;

    Terrain(const TerrainData *terrain_data, InteractableManager *interactable_manager, Camera *camera, vec3 pos = vec3(0.f)) :
        terrain_shader(ShaderManager::get_default_world_shader()),
        elevation_line_drawer(terrain_data->heightmap_path, terrain_data->vertical_scale),
        terrain_data(terrain_data), heightmap_texture(Texture(terrain_data->heightmap_path, 1))
    {
        // Setup the physical plane object
        terrain_obj = Plane(glm::max(terrain_data->resolution_x,terrain_data->resolution_y), pos);
        
        // Center the physical mesh so y=0 is the base
        terrain_obj.move(V3_Y * -(terrain_data->vertical_scale * 2)); 
        
        // generate and apply a colour texture
        TerrainPainter painter = TerrainPainter(terrain_data);
        Texture generated_texture = painter.bake_terrain_texture();

        // get interactable and name tag positions from painer, attach them to terrain
        vector<vec2> interactable_positions = painter.get_interactable_positions();
        for (vec2 uv : interactable_positions) {
            Interactable i = Interactable(vec3(0.f),INTERACTABLE_INTERACT_DISTANCE);
            interactable_manager->add ( &i );
            attach_to_surface (&i, uv.x, uv.y);
        }

        // handle shader and camera 
        terrain_shader = ShaderManager::get_terrain_shader(heightmap_texture, terrain_data->vertical_scale, generated_texture);
        terrain_shader.config_worldpos_buffer();
        camera->set_orthographic(&terrain_shader);
        terrain_obj.set_shader(&terrain_shader);
        
        // Standard Orientation: Rotated -45 degrees on X, Scaled up 3x
        terrain_obj.rotate(vec3(-45.f, 0.f, 0.f));
        terrain_obj.scale(3.f);
    }
    
    Plane* get_obj() {
        return &terrain_obj;
    }

    // --- Fixed Attach Function ---
    // Attaches an object to the terrain surface at specific UV coordinates (0.0 to 1.0)
    void attach_to_surface(Object *obj, float along_x, float along_y) {
        if (!obj) return;

        along_x = glm::clamp(along_x, 0.f, 1.f);
        along_y = glm::clamp(along_y, 0.f, 1.f);

        vec3 local_pos = elevation_line_drawer.get_local_pos_from_uv(along_x, along_y); // get local terrain position
        obj->set_position(local_pos); // reposition obj to local space (relative to terrain)
        obj->set_parent(&terrain_obj); // attached objects will follow terrain rotations
    }
};

#endif