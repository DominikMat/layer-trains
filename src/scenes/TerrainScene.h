#ifndef TERRAINSCENE_H
#define TERRAINSCENE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Scene.h"
#include "MatchSlopePath.h"
#include "AutoSlopePath.h"
#include "StraightPath.h"
#include "ButtonPanel.h"

enum ButtonID {
    MODE_STRAIGHT_PATH=0, MODE_ISO_PATH=2, MODE_AUTO_SLOPE=1
};

class TerrainScene : public Scene
{
public:
    Terrain *terrain;
    Plane *terrain_obj;
    InteractableManager *interactable_manager;

    TerrainPath *terrain_path_drawer[3];

    float last_scroll_value = 1.f;
    int current_path_draw_mode = ButtonID::MODE_STRAIGHT_PATH;

    /* testing */
    Text *test_text;
    Panel *test_panel;
    Interactable *test_interact;
    Text *slope_text;
    Panel *slope_panel;
    /* end testing */

    TerrainScene (TerrainData terrain_data, World *w, Camera *c, ScreenUI *s, World *wui, InputHandler *ih) : Scene(w,c,s,wui,ih) {
        interactable_manager = new InteractableManager(world, 
            [this](Interactable *i) { 
                this->interact_callback(i); 
            }
        );
        screen_ui->set_button_click_callback( 
            [this](int button_id, bool state) { 
                this->on_ui_button_clicked(button_id, state); 
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
        //terrain_path_drawer = new MatchSlopePath(terrain, world, 15.f, true);
        terrain_path_drawer[ButtonID::MODE_STRAIGHT_PATH] = new StraightPath(terrain, world, true);
        terrain_path_drawer[ButtonID::MODE_AUTO_SLOPE] = new AutoSlopePath(terrain, world, 1.f, true);
        terrain_path_drawer[ButtonID::MODE_ISO_PATH] = new MatchSlopePath(terrain, world, 0.25f, true);

        // ==========================================================
        /* Create ui */
        init_ui();
    }

    void loop(float dt) override {
        camera_controls(dt);

        // update terrain path
        terrain_path_drawer[current_path_draw_mode]->update_path(user_input);

        // process interactable objects
        vec3 mouse_terrain_local_pos = vec3(glm::inverse(terrain_obj->get_transform()) * vec4(user_input->get_mouse_position_world(), 1.f));
        interactable_manager->process_all(mouse_terrain_local_pos, user_input->is_left_mouse_clicked());

        // check end drawing
        if (user_input->is_left_mouse_clicked() && terrain_path_drawer[current_path_draw_mode]->is_drawing_path() 
            && glm::length(mouse_terrain_local_pos-terrain_path_drawer[current_path_draw_mode]->origin_point) > INTERACTABLE_INTERACT_DISTANCE) {
            terrain_path_drawer[current_path_draw_mode]->end_drawing_at_pos(mouse_terrain_local_pos);
            create_path_handle_at_pos (terrain_path_drawer[current_path_draw_mode]->get_end_point());
        }

        /* slope value display */
        if (current_path_draw_mode != ButtonID::MODE_STRAIGHT_PATH)
            slope_text->set_text((std::string)(current_path_draw_mode == ButtonID::MODE_AUTO_SLOPE ? "max " : "") + "slope: " + std::to_string((int)(terrain_path_drawer[current_path_draw_mode]->slope*100.f)) + "%");
        else
            slope_text->set_text("");
        
        // prints
        if (user_input->is_left_mouse_double_clicked()) std::cout << "Left Mouse DOUBLE clicked" << std::endl;
        if (user_input->is_middle_mouse_double_clicked()) std::cout << "middle Mouse DOUBLE clicked" << std::endl;
        if (user_input->is_right_mouse_double_clicked()) std::cout << "right Mouse DOUBLE clicked" << std::endl;
    }

    Shader* get_world_pos_buffer_shader() override {
        return terrain->terrain_shader;
    }

    void interact_callback (Interactable *interactable) {
        switch (interactable->type) {
            case InteractionType::PATH_HANDLE:
                if (!terrain_path_drawer[current_path_draw_mode]->is_drawing_path()) {
                    terrain_path_drawer[current_path_draw_mode]->start_drawing_at_pos(interactable->position);
                    //user_input->reset_scroll_value();
                    interactable->disable();
                }
                else {
                    //user_input->reset_scroll_value();
                    terrain_path_drawer[current_path_draw_mode]->end_drawing_at_pos(interactable->position);
                    last_scroll_value = user_input->get_scroll_value();
                }
                break;
            
            default:
                std::cout << "Interaction logged!" << std::endl;
                break;
        }
    }

    void on_ui_button_clicked (int button_id, bool clicked) {
        ButtonID id = (ButtonID) button_id;

        switch (id) {
            case ButtonID::MODE_STRAIGHT_PATH:
            case ButtonID::MODE_AUTO_SLOPE:
            case ButtonID::MODE_ISO_PATH:
                current_path_draw_mode = (int)id; 
                terrain_path_drawer[current_path_draw_mode]->reset(); 
                break;
        }

        std::cout << "BUTTON NR " << button_id << " SET TO STATE: " << clicked << std::endl;
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
        if (!terrain_path_drawer[current_path_draw_mode]->is_drawing_path()){
            float delta_scroll = user_input->get_scroll_value() - last_scroll_value;
            last_scroll_value = user_input->get_scroll_value();
            camera->change_orthographic_zoom(delta_scroll); 
        }

        /* Blender-like camera movement */
        if (user_input->is_right_mouse_held()){
            vec2 mouse_delta = user_input->get_mouse_movement_since_last_frame();
            float zoom_level_modifier = glm::clamp(camera->get_current_orthographic_zoom(), 0.2f, 1.75f);

            if (!user_input->is_holding_shift()) {
                camera->rotate(V3_Y * mouse_delta.x * CAMERA_ROTATION_SPEED * dt * zoom_level_modifier);
                camera->rotate(V3_X * mouse_delta.y * CAMERA_ROTATION_SPEED * dt * zoom_level_modifier);
            } else {
                camera->move(V3_X * mouse_delta.x * CAMERA_MOVEMENT_SPEED * dt * zoom_level_modifier);
                camera->move(V3_Y * -mouse_delta.y * CAMERA_MOVEMENT_SPEED * dt * zoom_level_modifier);
            }
        }
    }

    void init_ui() {
        
        /* Title */
        test_panel = new Panel(Colour::DARK_GREY, vec2(0), vec2(800, 85));
        test_panel->set_anchor( UIAnchor::TOP_LEFT, vec2(10,-10) );
        screen_ui->place( test_panel );
        test_text = new Text("Layer Trains Prototype ;)", 1.5f, Colour::WHITE);
        test_text->set_parent(test_panel);
        test_text->set_anchor( UIAnchor::BOTTOM_LEFT, vec2(15) );
        test_text->set_colour(Colour::WHITE);
        screen_ui->place( test_text );
        
        /* Slope display text */
        slope_panel = new Panel(Colour::DARK_GREY, vec2(0), vec2(400, 85));
        slope_panel->set_anchor( UIAnchor::BOTTOM_LEFT, vec2(30,30) );
        screen_ui->place( slope_panel );
        slope_text = new Text("Slope: ---%", 1.5f, Colour::WHITE);
        slope_text->set_parent(slope_panel);
        slope_text->set_anchor( UIAnchor::BOTTOM_LEFT, vec2(15) );
        slope_text->set_colour(Colour::WHITE);
        screen_ui->place( slope_text );

        /* button panel */
        ButtonPanel* toolbar = new ButtonPanel(vec2(400, 100), 50.0f, 10.0f, vec4(0.2f, 0.2f, 0.2f, 1.0f), Colour::WHITE);        
        toolbar->set_anchor(UIAnchor::BOTTOM_CENTER, vec2(0,30));
        toolbar->add_button(ButtonID::MODE_STRAIGHT_PATH, false, Colour::PINK);
        toolbar->add_button(ButtonID::MODE_AUTO_SLOPE, false, Colour::PURPLE);
        toolbar->add_button(ButtonID::MODE_ISO_PATH, false, Colour::SKY_BLUE);
        screen_ui->place(toolbar);
    }
};

#endif