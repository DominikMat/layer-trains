#ifndef TitleCardSCENE_H
#define TitleCardSCENE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "InputHandler.h"
#include "Scene.h"
#include "UIList.h"
#include "TextButton.h"
#include "BezierLine2D.h"

class TitleCardScene : public Scene
{
private:
    const int level_number = 1 + SceneID::LEVEL10 - SceneID::LEVEL1;
    enum ButtonID {
        LEVEL_SELECT, CREDITS, MAP_EDITOR_START, 
        LEVEL_START=100
    };

    UIList *menu_list;
    UIText *title_display;
    TextButton *credits_text;
    TextButton *level_select_button;
    TextButton* map_editor_button;

    BezierLine2D *bezier;
    UIText *bezier_text;

    UIList* level_select_panel;

    bool end_scene_flag = false;
    float end_scene_timer = 0.5;
    int next_scene_id = 0;

public:
    TitleCardScene (World *w, Camera *c, ScreenUI *s, InputHandler *ih) : Scene(w,c,s,ih) {
    }
    
    void init() override {
        screen_ui->set_button_click_callback( 
            [this](int button_id, bool state) { 
                this->on_ui_button_clicked(button_id, state); 
            }
        );

        set_background_colour(Colour::GREY);
        
        /* main menu list*/
        menu_list = new UIList(20, Colour::DARK_GREY, 20);
        menu_list->set_anchor( UIAnchor::TOP_LEFT, vec2(20,-20) );
        title_display = new UIText("Layer Trains Prototype ;)", 1.15f, Colour::WHITE);
        level_select_button = new TextButton("level select", 0.75f, Colour::WHITE, ButtonID::LEVEL_SELECT,true);
        map_editor_button = new TextButton("map editor", 0.75f, Colour::WHITE, ButtonID::MAP_EDITOR_START,false);
        credits_text = new TextButton("credits", 0.75f, Colour::WHITE, ButtonID::CREDITS,false);
        menu_list->add_item( title_display );
        menu_list->add_item( level_select_button );
        menu_list->add_item( map_editor_button );
        menu_list->add_item( credits_text );
        screen_ui->place( menu_list );
        
        /* curvy text display */
        Curve2D* bezier_curve = new BezierLine2D(vec2(400), vec2(550,525), vec2(700, 500));
        bezier_text = new UIText("bezier curvy :)", .75f, Colour::WHITE);
        bezier_text->place_on_curve(bezier_curve);
        screen_ui->place( bezier_text );
        
        /* Level select right panel */
        level_select_panel = new UIList(20, Colour::DARK_GREY, 20);
        level_select_panel->set_anchor(UIAnchor::MIDDLE_RIGHT, vec2(-250, 0));
        for (int i=0; i<level_number; i++) {
            std::string level_name = "level " + std::to_string(i+1);
            TextButton* lvl_btn = new TextButton(level_name.c_str(), 0.75f, Colour::WHITE, ButtonID::LEVEL_START+i,false);
            level_select_panel->add_item( lvl_btn );
        }
        level_select_panel->set_visible(false, true);
        level_select_panel->set_animation(AnimationPreset::ENTRY_RIGHT, AnimationPlace::INTRO);
        level_select_panel->set_animation(AnimationPreset::EXIT_RIGHT, AnimationPlace::OUTRO);
        screen_ui->place( level_select_panel );
    }
    
    void loop(float dt) override {
        /* end scene logic */
        if (end_scene_flag) end_scene_timer -= dt;
        if (end_scene_timer <= 0.f) end_scene(next_scene_id);
    }

    void on_ui_button_clicked(int button_id, bool state) {
        std::cout << "BUTTON NR " << button_id << " SET TO STATE: " << state << std::endl;

        switch(button_id) {
            case ButtonID::LEVEL_SELECT:
                level_select_panel->set_visible(state);
                level_select_button->set_text(state ? "choose level" : "level select");
                menu_list->recalculate_layout();
                break;
        
            case ButtonID::CREDITS:
                credits_text->set_text("made by Dominik Mat <3");
                menu_list->recalculate_layout();
                break;
        
            case ButtonID::MAP_EDITOR_START:
                map_editor_button->set_text("loading ...");
                menu_list->recalculate_layout();
                end_scene_flag = true;
                next_scene_id = SceneID::MAP_EDITOR;
                break;       
        }

        /* progress to specific level */
        if (button_id >= ButtonID::LEVEL_START && button_id < ButtonID::LEVEL_START+level_number) {
            level_select_panel->set_visible(false);
            end_scene_flag = true;
            next_scene_id = SceneID::LEVEL1 + (button_id-ButtonID::LEVEL_START);
        }
    }
};

#endif