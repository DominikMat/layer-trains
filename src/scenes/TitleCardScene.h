#ifndef TitleCardSCENE_H
#define TitleCardSCENE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "InputHandler.h"
#include "Scene.h"
#include "UIList.h"
#include "TextButton.h"

class TitleCardScene : public Scene
{
private:
    enum ButtonID {
        PROGRAM_START=0, CREDITS=1
    };

    UIList *menu_list;
    UIText *title_display;
    TextButton *credits_text;
    TextButton *start_program_button;

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
        
        menu_list = new UIList(20, Colour::DARK_GREY, 20);
        menu_list->set_anchor( UIAnchor::TOP_LEFT, vec2(20,-20) );
        menu_list->add_item( new UIText("Layer Trains Prototype ;)", 1.15f, Colour::WHITE) );
        menu_list->add_item( new TextButton("start", 0.75f, Colour::WHITE, ButtonID::PROGRAM_START,true) );
        credits_text = new TextButton("credits", 0.75f, Colour::WHITE, ButtonID::CREDITS,false);
        menu_list->add_item( credits_text );
        screen_ui->place( menu_list );

    }
    
    void loop(float dt) override {
    }

    void on_ui_button_clicked(int button_id, bool state) {
        std::cout << "BUTTON NR " << button_id << " SET TO STATE: " << state << std::endl;

        if (button_id == ButtonID::PROGRAM_START) end_scene();
        if (button_id == ButtonID::CREDITS) {
            credits_text->set_text("made by Dominik Mat <3");
            menu_list->recalculate_layout();
        }
    }
};

#endif