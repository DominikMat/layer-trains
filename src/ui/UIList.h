#ifndef UILIST_H
#define UILIST_H

#include <vector>
#include <algorithm>
#include "UIObject.h"
#include "Panel.h"

class Button;

class UIList : public UIObject {
private:
    float gap_size, padding;
    std::vector<UIObject*> items;
    std::vector<Button*> buttons;
    
    bool has_background;
    vec4 background_color;
    Panel* background_panel;

public:
    UIList(float gap, vec4 bg_col, float padding_px, vec2 pos = vec2(0.f))
        : UIObject(pos, vec2(10.f, 10.f)), // Rozmiar początkowy, zaktualizuje się sam
          gap_size(gap), background_color(bg_col), padding(padding_px)
    {
        has_background = (bg_col.a > 0.0f);
        this->set_anchor(UIAnchor::CENTER, vec2(0.f)); // Domyślna kotwica

        if (has_background) {
            background_panel = new Panel(background_color, vec2(0.f), vec2(0.f));
            background_panel->set_parent(this);
            background_panel->set_screenspace();
            background_panel->set_anchor(UIAnchor::CENTER, vec2(0.f));
        }

    }

    void add_item(UIObject* item) {
        if (!item) return;

        item->set_parent(this);
        item->set_screenspace();
        
        items.push_back(item);

        if (this->shader) {
            item->construct();
            item->set_shader(this->shader);
        }

        recalculate_layout();

        std::vector<Button*> item_buttons = item->get_buttons();
        for (auto* b : item_buttons) buttons.push_back(b);
    }

    void recalculate_layout() {
        if (items.empty()) {
            set_size(vec3(0.f));
            return;
        }

        float total_height = 0.f;
        float max_width = 0.f;

        for (auto* item : items) {
            total_height += item->size.y;
            if (item->size.x > max_width) max_width = item->size.x;
        }

        total_height += (items.size() - 1) * gap_size;
        set_size(vec3(max_width+padding*2.f, total_height+padding*2.f, 1.f));
        this->recalculate_ui_position(); 

        if (has_background) {
            background_panel->set_size(size);
            background_panel->recalculate_ui_position();
        }

        float current_y = padding;
        for (auto* item : items) {
            item->set_anchor(UIAnchor::TOP_LEFT, vec2(padding, -current_y));
            current_y += (item->size.y + gap_size);
        }
    }
    
    void construct() override {
        for (auto* item : items) {
            item->construct();
        }
        if (has_background) background_panel->construct();
    }

    void set_shader(Shader* s) override {
        Object::set_shader(s);
        for (auto* item : items) {
            item->set_shader(s);
        }
        if (has_background) background_panel->set_shader(s);
    }
    
    void calculate_transform_matrix() override {
        Object::calculate_transform_matrix(); 
        for (auto* item : items) {
            item->calculate_transform_matrix();
        }
        if (has_background) background_panel->calculate_transform_matrix();
    }

    void render() override {
        if (!visible) return;
        
        if (has_background) {
            background_panel->enable_shader();
            background_panel->update_transform();
            background_panel->configure_render_properties();
            background_panel->render();
            background_panel->disable_render_properties();
        }

        for (auto* item : items) {
            item->enable_shader();
            item->update_transform();
            item->configure_render_properties();
            item->render();
            item->disable_render_properties();
        }
    }

    void clear_items() {
        for (auto* item : items) delete item;
        items.clear();
        recalculate_layout();
    }
    
    void resize_and_reposition() override {
        UIObject::resize_and_reposition();
        for(auto* item : items) {
            item->recalculate_ui_position();
        }
    }

    std::vector<UIObject*> get_items() { return items; }
    std::vector<Button*> get_buttons() override { return buttons; }
    UIObject* get_item(int i) { return i > 0 && i <items.size() ? items[i] : nullptr; }

    ~UIList() override {
        for (auto* item : items) delete item;
        if (has_background) delete background_panel;
    }
};

#endif