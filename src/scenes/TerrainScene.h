#ifndef TERRAINSCENE_H
#define TERRAINSCENE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <glm/glm.hpp>
#include "Scene.h"
#include "MatchSlopePathDrawer.h"
#include "TerrainPath.h"
#include "AutoSlopePathDrawer.h"
#include "PathSystem.h"
#include "StraightPathDrawer.h"
#include "ToolbarPanel.h"
#include "TextPanel.h"
#include "UIRow.h"

#define curr_path_drawer terrain_path_drawer[current_path_draw_mode]


class TerrainScene : public Scene
{
private:
    enum ButtonID {
        MODE_STRAIGHT_PATH=0, MODE_AUTO_SLOPE=1, MODE_ISO_PATH=2,
        CLICK_MENU_CREATE_PATH_HANDLE = 10, CLICK_MENU_CREATE_BRIDGE = 11, CLICK_MENU_CREATE_TUNNEL = 12,  CLICK_MENU_DRAG_PATH = 13, CLICK_MENU_DELETE_PATH = 14, 
        TOOL_BRIDGE=20,TOOL_TUNNEL=21,TOOL_RAIL=22,TOOL_ROAD=23
    };
    
    InteractableManager *interactable_manager;
    
    Terrain *terrain;   
    Plane *terrain_obj;
    const TerrainData *terrain_data;
    TerrainPathDrawer *terrain_path_drawer[3];
    
    float last_scroll_value = 1.f;
    int current_path_draw_mode = ButtonID::MODE_STRAIGHT_PATH;
    int draw_start_handle_id = 0;

    PathSystem *path_system;
    TextPanel *slope_display;
    Interactable *test_interact;

    UIList *click_menu;
    UIRow *toolbar_row;
    vec3 reference_pos_terrain_local;

public:

    TerrainScene (const TerrainData *terrain_data, World *w, Camera *c, ScreenUI *s, InputHandler *ih) : Scene(w,c,s,ih), terrain_data(terrain_data) {
    }
    
    void init( ) override {
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
        terrain = new Terrain(terrain_data, world, interactable_manager, camera);

        // configure terrain object
        terrain_obj = terrain->get_obj();

        // --- Interaction Objects ---
        test_interact = new Interactable(vec3(0.f), "test interact", InteractionType::PATH_HANDLE, INTERACTABLE_INTERACT_DISTANCE); // Position 0, will be moved by attach
        terrain->attach_to_surface( test_interact, 0.5f, 0.5f ); 
        interactable_manager->add(test_interact);
        
        // --- Path drawer ---
        terrain_path_drawer[ButtonID::MODE_STRAIGHT_PATH] = new StraightPathDrawer(terrain, world, true);
        terrain_path_drawer[ButtonID::MODE_AUTO_SLOPE] = new AutoSlopePathDrawer(terrain, world, 1.f, true);
        terrain_path_drawer[ButtonID::MODE_ISO_PATH] = new MatchSlopePathDrawer(terrain, world, 0.25f, true);

        // --- config path system ----
        path_system = new PathSystem();
        for (auto i : interactable_manager->get_current_interactables()) {
            if (i->type == InteractionType::PATH_HANDLE) { path_system->create_destination(i, false);
            std::cout << "added destination: " << i->name << std::endl; }
        }

        // ==========================================================
        /* Create ui */
        init_ui();
    }

    void loop(float dt) override {
        camera_controls(dt);

        // update terrain path'
        curr_path_drawer->update_path(user_input);

        // process interactable objects
        vec3 mouse_pos_world = user_input->get_mouse_position_world();
        vec3 mouse_terrain_local_pos = vec3(glm::inverse(terrain_obj->get_transform()) * vec4(mouse_pos_world, 1.f));
        interactable_manager->process_all(mouse_terrain_local_pos, user_input->is_left_mouse_pressed_up());
        interactable_manager->resize_on_zoom(camera->get_current_orthographic_zoom()); 
        
        // check end drawing
        if (user_input->is_left_mouse_pressed_up() && curr_path_drawer->is_drawing_path() 
            && glm::length(mouse_terrain_local_pos-curr_path_drawer->origin_point) > INTERACTABLE_INTERACT_DISTANCE) {
            Interactable* i = create_path_handle_at_pos (vec3(curr_path_drawer->get_end_point(),0.f));
            if (i) { 
                path_system->create_destination(i,true); 
                path_system->add_link(draw_start_handle_id, i->get_id(), 10.f);
                curr_path_drawer->end_drawing_at_pos(mouse_terrain_local_pos);
                //draw_start_handle_id = i->get_id();
                //curr_path_drawer->start_drawing_at_pos(end_pos);
            }
        }
        
        /* slope value display */
        bool display_slope_info = current_path_draw_mode != ButtonID::MODE_STRAIGHT_PATH;
        if (display_slope_info) slope_display->set_text((std::string)(current_path_draw_mode == ButtonID::MODE_AUTO_SLOPE ? "max " : "") + "slope: " + std::to_string((int)(curr_path_drawer->slope*100.f)) + "%");
        slope_display->set_visible(display_slope_info);
    
        /* click menu logic */
        if ((user_input->is_right_mouse_double_clicked() || user_input->is_left_mouse_double_clicked()) && !click_menu->visible) {
            click_menu_change_visible(true, mouse_pos_world, mouse_terrain_local_pos);
        } else if ((user_input->is_left_mouse_pressed_down() || user_input->is_right_mouse_pressed_down()) && click_menu->visible) {
            if (!click_menu->is_mouse_over(user_input->get_mouse_position_pixels_inv_y())) click_menu_change_visible(false);
        }
    }

    Shader* get_world_pos_buffer_shader() override {
        return terrain->terrain_shader;
    }

    void interact_callback (Interactable *interactable) {
        switch (interactable->type) {
            case InteractionType::PATH_HANDLE:
                if (!curr_path_drawer->is_drawing_path()) {
                    curr_path_drawer->start_drawing_at_pos(interactable->position);
                    //user_input->reset_scroll_value();
                    interactable->disable();
                    draw_start_handle_id = interactable->get_id();
                }
                else {
                    //user_input->reset_scroll_value();
                    curr_path_drawer->end_drawing_at_pos(interactable->position);
                    last_scroll_value = user_input->get_scroll_value();
                    path_system->add_link(draw_start_handle_id,interactable->get_id(),10.f); // TODO len calc

                    std::cout << "NEW DESTINATION CONNECTED! :))) " << interactable->name << std::endl;
                    if(path_system->are_necessary_destinations_connected()) 
                        std::cout << "ALL DESTINATIONS REACHED! :]]]]]]]]]]]]]]]]]]]]] " << std::endl;
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
                break;

            case ButtonID::CLICK_MENU_CREATE_PATH_HANDLE:
                create_path_handle_at_pos(reference_pos_terrain_local);
                click_menu_change_visible(false);
                break;
        }
        terrain_path_drawer[current_path_draw_mode]->reset(); 

        std::cout << "BUTTON NR " << button_id << " SET TO STATE: " << clicked << std::endl;
    }

    Interactable* create_path_handle_at_pos (vec3 local_pos) {
        Interactable *i = interactable_manager->create(
            vec3(0.f), "Path Handle", InteractionType::PATH_HANDLE, INTERACTABLE_INTERACT_DISTANCE
        );
        float uvx = (local_pos.x+0.5f); // local -0.5 to 0.5, uv is 0-1
        float uvy = (local_pos.y+0.5f); // local -0.5 to 0.5, uv is 0-1
        terrain->attach_to_surface(i, uvx, uvy);
        cout << "Path handle created, attached to surface at: " << uvx << ", " << uvy << endl;
        return i;
    }

private:
    void camera_controls(float dt) {
        /* Zoom control */
        if (!curr_path_drawer->is_drawing_path()){
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
        /* Slope display text */
        slope_display = new TextPanel("Slope: ---%", 0.75f, Colour::WHITE, Colour::DARK_GREY, vec2(400, 85), true, true);
        slope_display->set_anchor( UIAnchor::BOTTOM_LEFT, vec2(30,30) );
        screen_ui->place( slope_display );

        /* toolbar row */
        toolbar_row = new UIRow(30, Colour::TRANSPARENT, 0);
        toolbar_row->set_anchor(UIAnchor::BOTTOM_CENTER, vec2(0,30));
        
        const float button_size = 35.f, button_gap = 5.f;
        const vec4 toolbar_bg = Colour::DARK_GREY;
        std::string icon_filepath = TEXTURE_ICON_FILE_PATH;

        ToolbarPanel* toolbar_left = new ToolbarPanel(button_gap, toolbar_bg, button_size); 
        toolbar_left->add_button(ButtonID::TOOL_BRIDGE, false, new Texture(((std::string)(TEXTURE_ICON_FILE_PATH)).append("/bridge_tool.png").c_str()));
        toolbar_left->add_button(ButtonID::TOOL_TUNNEL, false, new Texture(((std::string)(TEXTURE_ICON_FILE_PATH)).append("/tunnel_tool.png").c_str()));
        toolbar_row->add_item(toolbar_left);
        ToolbarPanel* path_toolbar = new ToolbarPanel(button_gap, toolbar_bg, button_size);
        path_toolbar->add_button(ButtonID::MODE_STRAIGHT_PATH, false, new Texture(((std::string)(TEXTURE_ICON_FILE_PATH)).append("/straight_tool.png").c_str()));
        path_toolbar->add_button(ButtonID::MODE_AUTO_SLOPE, false, new Texture(((std::string)(TEXTURE_ICON_FILE_PATH)).append("/auto_slope_tool.png").c_str()));
        path_toolbar->add_button(ButtonID::MODE_ISO_PATH, false, new Texture(((std::string)(TEXTURE_ICON_FILE_PATH)).append("/match_slope_tool.png").c_str()));
        toolbar_row->add_item(path_toolbar);
        ToolbarPanel* toolbar_right = new ToolbarPanel(button_gap, toolbar_bg, button_size);
        toolbar_right->add_button(ButtonID::TOOL_ROAD, false, new Texture(((std::string)(TEXTURE_ICON_FILE_PATH)).append("/road_tool.png").c_str()));
        toolbar_right->add_button(ButtonID::TOOL_RAIL, false, new Texture(((std::string)(TEXTURE_ICON_FILE_PATH)).append("/rail_tool.png").c_str()));
        toolbar_row->add_item(toolbar_right);
        screen_ui->place(toolbar_row);

        /* click menu */
        click_menu = new UIList(0, Colour::DARK_GREY, 5);
        const float click_menu_font_size = 0.3f;
        click_menu->add_item( new TextButton("create path handle", click_menu_font_size, Colour::WHITE, ButtonID::CLICK_MENU_CREATE_PATH_HANDLE, false) );
        click_menu->add_item( new TextButton("create tunnel", click_menu_font_size, Colour::WHITE, ButtonID::CLICK_MENU_CREATE_BRIDGE, false) );
        click_menu->add_item( new TextButton("create bridge", click_menu_font_size, Colour::WHITE, ButtonID::CLICK_MENU_CREATE_TUNNEL, false) );
        click_menu->add_item( new TextButton("drag path segment", click_menu_font_size, Colour::WHITE, ButtonID::CLICK_MENU_DRAG_PATH, false) );
        click_menu->add_item( new TextButton("delete path", click_menu_font_size, Colour::WHITE, ButtonID::CLICK_MENU_DELETE_PATH, false) );
        click_menu->set_visible(false);
        screen_ui->place( click_menu );
    }

    void click_menu_change_visible(bool state, vec3 mouse_world=vec3(0), vec3 mouse_local=vec3(0)) {
        // enable
        if (state){
            vec2 mouse_pos_px = user_input->get_mouse_position_pixels(); mouse_pos_px.y *= -1;
            click_menu->set_anchor(UIAnchor::TOP_LEFT, mouse_pos_px + vec2(5,-5));
            click_menu->recalculate_layout();
            click_menu->set_visible(true);
            if (terrain->terrain_shader) {
                terrain->terrain_shader->use();
                terrain->terrain_shader->setVec3("reference_point_pos", mouse_world);
                terrain->terrain_shader->setBool("draw_reference", true);
                reference_pos_terrain_local = mouse_local;
            }
        }
        // disable
        if (!state) { 
            click_menu->set_visible(false);
            if (terrain->terrain_shader) {
                terrain->terrain_shader->use();
                terrain->terrain_shader->setBool("draw_reference", false);
            }
        }
    }
};

#endif