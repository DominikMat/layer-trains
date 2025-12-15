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
    vector<UIObject*> items;
    bool has_background = false;
    Panel* background_panel = nullptr;

public:
    UIList(float gap, vec4 bg_col, float padding_px, vec2 pos = vec2(0.f))
        : UIObject(pos, vec2(10.f, 10.f)), // Rozmiar początkowy, zaktualizuje się sam
          gap_size(gap), padding(padding_px)
    {
        this->set_anchor(UIAnchor::CENTER, vec2(0.f)); // Domyślna kotwica
        has_background = bg_col.a > 0.0f;
        
        if (has_background) {
            background_panel = new Panel(bg_col, vec2(0.f), vec2(0.f));
            background_panel->set_ui_parent(this);
            background_panel->set_screenspace();
            background_panel->set_anchor(UIAnchor::CENTER, vec2(0.f));
        }
    }

    void add_item(UIObject* item) {
        if (!item) return;
        item->set_ui_parent(this);
        items.push_back(item);
        recalculate_layout();
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

    void construct() override {} // nothing, only children construct
    void render() override {} // nothing only children render

    void resize_and_reposition() override {
        recalculate_layout();
    }
};

#endif