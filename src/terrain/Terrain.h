#ifndef TERRAIN_H
#define TERRAIN_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <regex>
#include "world_objects/Object.h"
#include "world_objects/Plane.h"
#include "TerrainPlane.h"
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

class Terrain
{
public:
    TerrainPlane *terrain_obj;
    Plane *terrain_floor;
    Shader *terrain_shader = nullptr;
    ElevationLineDrawer elevation_line_drawer;
    const TerrainData *terrain_data;
    Texture heightmap_texture;

    vector<Interactable*> attached_interactables;

    Terrain(const TerrainData *terrain_data, World *w, InteractableManager *interactable_manager, Camera *camera, vec3 pos = vec3(0.f)) :
        //terrain_shader(new DEFAULT_WORLD_SHADER),
        elevation_line_drawer(terrain_data->heightmap_path, terrain_data->vertical_scale),
        terrain_data(terrain_data), heightmap_texture(Texture(terrain_data->heightmap_path, true, true))
    {
        // Setup the physical plane object for terrain and floor
        terrain_obj = new TerrainPlane(terrain_data, camera, pos);

        terrain_floor = new Plane(2, pos);
        terrain_floor->set_parent(terrain_obj);
        terrain_floor->set_colour(Colour::DARK_GREY);
        //terrain_floor->set_colour(Colour::TERRAIN_SIDE_COLOUR);
        
        // Center the physical mesh so y=0 is the base
        terrain_obj->move(V3_Y * -(terrain_data->vertical_scale * 2)); 
        
        // generate and apply a colour texture
        TerrainPainter painter = TerrainPainter(terrain_data);
        Texture generated_texture = painter.bake_terrain_texture();

        // get interactable and name tag positions from painer, attach them to terrain
        //vector<vec2> interactable_positions = painter.get_interactable_positions();
        for (TerrainTag tag : terrain_data->tags) {
            if (tag.type == TerrainTagType::DISABLED) break; // reached unused tag space, can exit
            
            // select interaction type and distance based on tag type
            InteractionType interaction_type = InteractionType::NONE;;
            float interact_dist = 0.f;

            switch (tag.type) {
                // path handles
                case TerrainTagType::LEVEL_START:
                case TerrainTagType::LEVEL_END:
                    interaction_type = InteractionType::PATH_HANDLE;
                    interact_dist = INTERACTABLE_INTERACT_DISTANCE;
                    break;

                // no interaction
                case TerrainTagType::NAME_TAG:
                default:
                    interaction_type = InteractionType::NONE;
                    interact_dist = INTERACTABLE_INTERACT_DISTANCE;
            }

            // create interactable object in interactable manager, attach it to the terrain
            Interactable *i = interactable_manager->create ( vec3(0.f), tag.name, interaction_type, interact_dist );
            attach_to_surface (i, tag.uv_x, tag.uv_y);
            cout << "Interactable " << tag.name << " detected, attached to surface at: " << tag.uv_x << ", " << tag.uv_y << endl;

            if (tag.type == TerrainTagType::NAME_TAG) { // create world space text name tag 
                Text *name_tag_obj = new Text(tag.name, 1.5f/SCR_WIDTH, Colour::BLACK);
                name_tag_obj->set_shader(new WORLD_UI_SHADER);
                attach_to_surface(name_tag_obj, tag.uv_x, tag.uv_y);
                //name_tag_obj->set_size(0.01f);
                name_tag_obj->move(V3_Z*0.1f);
                name_tag_obj->move(-V3_X*name_tag_obj->get_text_width()/2.f);
                name_tag_obj->rotate(V3_X*90.f);
                name_tag_obj->set_colour(Colour::BLACK);
                w->place(name_tag_obj);
            }
        }

        // handle shader and camera 
        terrain_shader = &ShaderManager::get_terrain_shader(&heightmap_texture, terrain_data->vertical_scale, &generated_texture);
        terrain_shader->config_worldpos_buffer();
        camera->set_orthographic(terrain_shader);
        terrain_obj->set_shader(terrain_shader);
        
        // Standard Orientation: Rotated -45 degrees on X, Scaled up 3x
        terrain_obj->rotate(vec3(-90.f, 0.f, 0.f));
        terrain_obj->scale(3.f);

        // place objects in world
        w->place(terrain_obj);
        w->place(terrain_floor);
    }
    
    Plane* get_obj() {
        return terrain_obj;
    }

    // --- Fixed Attach Function ---
    // Attaches an object to the terrain surface at specific UV coordinates (0.0 to 1.0)
    void attach_to_surface(Object *obj, float along_x, float along_y) {
        if (!obj) return;

        along_x = glm::clamp(along_x, 0.f, 1.f);
        along_y = glm::clamp(along_y, 0.f, 1.f);

        vec3 local_pos = elevation_line_drawer.get_local_pos_from_uv(along_x, along_y); // get local terrain position
        obj->set_position(local_pos); // reposition obj to local space (relative to terrain)
        obj->set_parent(terrain_obj); // attached objects will follow terrain rotations
    }
};

#endif