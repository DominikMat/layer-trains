#ifndef TERRAINSCENE_H
#define TERRAINSCENE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Scene.h"

class TerrainScene : public Scene
{
public:
    Terrain *terrain;
    Plane *terrain_obj;
    InteractableManager *interactable_manager;

    TerrainPath *terrain_path_drawer;

    float last_scroll_value = 1.f;

    /* testing */
    Text *test_text;
    Interactable *test_interact;
    Panel *test_panel;
    /* end testing */

    TerrainScene (TerrainData terrain_data, World *w, Camera *c, ScreenUI *s, World *wui, InputHandler *ih) : Scene(w,c,s,wui,ih) {
        interactable_manager = new InteractableManager(world, 
            [this](Interactable *i) { 
                this->interact_callback(i); 
            }
        );
        terrain = new Terrain(&terrain_data, w, interactable_manager, camera);
    }

    void init( ) override {
        // configure terrain object
        terrain_obj = terrain->get_obj();

        // --- Interaction Objects ---
        test_interact = new Interactable(vec3(0.f), "test interact", InteractionType::PATH_HANDLE, INTERACTABLE_INTERACT_DISTANCE); // Position 0, will be moved by attach
        terrain->attach_to_surface( test_interact, 0.5f, 0.5f ); 
        interactable_manager->add(test_interact);
        
        // --- Path drawer ---
        terrain_path_drawer = new TerrainPath(terrain, world, 15.f, true);

        // ==========================================================
        /* Create ui */

        test_panel = new Panel(Colour::DARK_GREY, vec2(0), vec2(800, 85));
        test_panel->set_anchor( UIAnchor::TOP_LEFT, vec2(10,-10) );
        screen_ui->place( test_panel );

        test_text = new Text("Layer Trains Prototype ;)", 1.5f, Colour::WHITE);
        test_text->set_parent(test_panel);
        test_text->set_anchor( UIAnchor::BOTTOM_LEFT, vec2(15) );
        test_text->set_colour(Colour::WHITE);
        screen_ui->place( test_text );

    }

    void loop(float dt) override {
        camera_controls(dt);

        // update terrain path
        terrain_path_drawer->update_path(user_input);
        test_text->set_text("grade: " + std::to_string(terrain_path_drawer->get_current_grade()));

        // process interactable objects
        vec3 mouse_terrain_local_pos = vec3(glm::inverse(terrain_obj->get_transform()) * vec4(user_input->get_mouse_position_world(), 1.f));
        interactable_manager->process_all(mouse_terrain_local_pos, user_input->is_left_mouse_clicked());

        // check end drawing
        if (user_input->is_left_mouse_clicked() && terrain_path_drawer->drawing() && glm::length(mouse_terrain_local_pos-terrain_path_drawer->origin_point) > INTERACTABLE_INTERACT_DISTANCE) {
            terrain_path_drawer->end_drawing_at_pos(mouse_terrain_local_pos);
            create_path_handle_at_pos (terrain_path_drawer->end_point);
        }

        // prints
    
        if (user_input->is_left_mouse_double_clicked()) std::cout << "Left Mouse DOUBLE clicked" << std::endl;
        if (user_input->is_middle_mouse_double_clicked()) std::cout << "middle Mouse DOUBLE clicked" << std::endl;
        if (user_input->is_right_mouse_double_clicked()) std::cout << "right Mouse DOUBLE clicked" << std::endl;
    }

    Shader* get_world_pos_buffer_shader() override {
        return &terrain->terrain_shader;
    }

    void interact_callback (Interactable *interactable) {
        switch (interactable->type) {
            case InteractionType::PATH_HANDLE:
                if (!terrain_path_drawer->drawing()) {
                    terrain_path_drawer->start_drawing_at_pos(interactable->position);
                    user_input->reset_scroll_value();
                    interactable->disable();
                }
                else {
                    user_input->reset_scroll_value();
                    terrain_path_drawer->end_drawing_at_pos(interactable->position);
                }
                break;
            
            default:
                std::cout << "Interaction logged!" << std::endl;
                break;
        }
    }

    void create_path_handle_at_pos (vec3 local_pos) {
        Interactable *i = interactable_manager->create(
            vec3(0.f), "Path Handle", InteractionType::PATH_HANDLE, INTERACTABLE_INTERACT_DISTANCE
        );
        float uvx = (local_pos.x+0.5f); // local -0.5 to 0.5, uv is 0-1
        float uvy = (local_pos.y+0.5f); // local -0.5 to 0.5, uv is 0-1
        terrain->attach_to_surface(i, uvx, uvy);
        cout << "Interactable created, attached to surface at: " << uvx << ", " << uvy << endl;
    }

private:
    void camera_controls(float dt) {
        /* Zoom control */
        if (!terrain_path_drawer->drawing()){
            float delta_scroll = user_input->get_scroll_value() - last_scroll_value;
            last_scroll_value = user_input->get_scroll_value();
            camera->change_orthographic_zoom(delta_scroll); 
        }

        /* Blender-like camera movement */
        if (user_input->is_right_mouse_held()){
            vec2 mouse_delta = user_input->get_mouse_movement_since_last_frame();
            float zoom_level_modifier = glm::clamp(camera->get_current_orthographic_zoom(), 0.2f, 1.75f);

            if (!user_input->is_holding_shift()) {
                camera->rotate(V3_Y * mouse_delta.x * CAMERA_PAN_SPEED * dt * zoom_level_modifier);
                camera->rotate(V3_X * mouse_delta.y * CAMERA_PAN_SPEED * dt * zoom_level_modifier);
            } else {
                camera->move(V3_X * mouse_delta.x * CAMERA_MOVEMENT_SPEED * dt * zoom_level_modifier);
                camera->move(V3_Y * -mouse_delta.y * CAMERA_MOVEMENT_SPEED * dt * zoom_level_modifier);
            }
        }
    }
};

#endif