#ifndef UIRow_H
#define UIRow_H

#include <vector>
#include <algorithm>
#include "UIObject.h"
#include "Panel.h"
#include "Button.h"

class UIRow : public UIObject {
private:
    float gap_size, padding;
    vector<UIObject*> items;
    bool has_background = false;
    Panel* background_panel = nullptr;

public:
    UIRow(float gap, vec4 bg_col, float padding_px, vec2 pos = vec2(0.f))
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

        float max_height = 0.f;
        float total_width = 0.f;

        for (auto* item : items) {
            total_width += item->size.x;
            if (item->size.y > max_height) max_height = item->size.y;
        }

        total_width += (items.size() - 1) * gap_size;
        set_size(vec3(total_width+padding*2.f, max_height+padding*2.f, 1.f));
        this->recalculate_ui_position(); 

        if (has_background) {
            background_panel->set_size(size);
            background_panel->recalculate_ui_position();
        }

        float current_x = padding;
        for (auto* item : items) {
            item->set_anchor(UIAnchor::MIDDLE_LEFT, vec2(current_x, 0));
            current_x += (item->size.x + gap_size);
        }
    }

    void construct() override {} // nothing, only children construct
    void render() override {} // nothing only children render

    void resize_and_reposition() override {
        recalculate_layout();
    }
};

#endif