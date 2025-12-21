#ifndef UITEXT_H
#define UITEXT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "UIObject.h"
#include "Shader.h"
#include "BezierLine2D.h" // for Curve2D interface
  
struct Character {
    unsigned int TextureID;  
    glm::ivec2   Size;       
    glm::ivec2   Bearing;    
    unsigned int Advance;    
};

class UIText : public UIObject {
private:
    std::string textString;
    float font_scale; // Renamed to avoid confusion with Object::size (transform scale)
    unsigned int VAO, VBO;
    int height_below_writing_line = 0;
    
    static std::map<GLchar, Character> Characters;
    static bool isFontLoaded;

    bool on_curve = false;
    Curve2D* curve = nullptr;

public:
    UIText(std::string text, float font_scale = 1.0f, vec4 color = Colour::BLACK)
        : UIObject(vec2(0), vec2(1)), textString(text), font_scale(font_scale) {
        set_colour(color);
        this->uses_texture = true; 
        
        if (!isFontLoaded) {
            loadFont(DEFAULT_FONT); 
        }

        resize_and_reposition();
    }

    static void loadFont(const char* fontPath) {
        FT_Library ft;
        if (FT_Init_FreeType(&ft)) { std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl; return; }
        FT_Face face;
        if (FT_New_Face(ft, fontPath, 0, &face)) { std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl; return; }
        FT_Set_Pixel_Sizes(face, 0, 48);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 
        for (unsigned char c = 0; c < 128; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            Character character = { texture, glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows), glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top), (unsigned int)face->glyph->advance.x };
            Characters.insert(std::pair<GLchar, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0); 
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        isFontLoaded = true;
    }

    void construct() override {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void configure_render_properties() override {
        UIObject::configure_render_properties(); 

        glActiveTexture(GL_TEXTURE0);
        shader->setBool("isText", true);
    }

    void render() override {
        if (!can_render()) return;

        glBindVertexArray(VAO);

        // Cursor for standard linear text
        float cursor_x = -size.x/2.f;
        float cursor_y = -size.y / 2.0f + height_below_writing_line; 
        
        for (int i=0; i < textString.size(); i++) 
        {
            GLchar c = textString.at(i);
            Character ch = Characters[c];

            // 1. Calculate Local Offsets (relative to the anchor point of the letter)
            float w = ch.Size.x * font_scale;
            float h = ch.Size.y * font_scale;
            float x_rel = ch.Bearing.x * font_scale;
            float y_rel = (ch.Bearing.y - ch.Size.y) * font_scale;

            // Define the 4 corners relative to the specific character's anchor
            vec2 local_verts[4] = {
                { x_rel,     y_rel + h }, // TL
                { x_rel,     y_rel     }, // BL
                { x_rel + w, y_rel     }, // BR
                { x_rel + w, y_rel + h }  // TR
            };
            vec2 final_pos[4];

            // 2. Apply Transformation
            if (on_curve) {
                float t = (float)i / (textString.size() - 1);
                
                // Get Curve Data
                vec2 p = curve->get_curve_position(t) - vec2(size)/2.f;
                vec2 normal = curve->get_curve_normal(t);
                vec2 tangent = vec2(normal.y, -normal.x); // Rotate normal -90 deg

                // Rotate Local Offsets + Add Curve Position
                for(int k=0; k<4; k++) {
                    final_pos[k] = p + (tangent * local_verts[k].x) + (normal * local_verts[k].y);
                }
            } 
            else {
                // Standard Linear Placement
                for(int k=0; k<4; k++) {
                    final_pos[k] = vec2(cursor_x, cursor_y) + local_verts[k];
                }
            }

            // 3. Map to GL_TRIANGLES (using the transformed positions)
            float vertices[6][4] = {
                { final_pos[0].x, final_pos[0].y,   0.0f, 0.0f },
                { final_pos[1].x, final_pos[1].y,   0.0f, 1.0f },
                { final_pos[2].x, final_pos[2].y,   1.0f, 1.0f },

                { final_pos[0].x, final_pos[0].y,   0.0f, 0.0f },
                { final_pos[2].x, final_pos[2].y,   1.0f, 1.0f },
                { final_pos[3].x, final_pos[3].y,   1.0f, 0.0f }
            };

            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            cursor_x += (ch.Advance >> 6) * font_scale; 
        }
        
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    void disable_render_properties() override {
        shader->setBool("isText", false);   
    }

    void set_text(std::string newText) {
        textString = newText;
        resize_and_reposition();
    }

    float get_text_width() {
        float width = 0;
        for (char c : textString) {
            width += (Characters[c].Advance >> 6) * font_scale;
        }
        return width;
    }
    float get_text_max_height() {
        int max_ascent = 0, max_descent = 0;
        for (char c : textString) {
            Character ch = Characters[c];

            // height above writing line
            if (ch.Bearing.y > max_ascent) { max_ascent = ch.Bearing.y; }

            // height below writing line
            int descent = ch.Size.y - ch.Bearing.y;
            if (descent > max_descent) { max_descent = descent; }
        }
        height_below_writing_line = max_descent * font_scale;
        return (max_ascent + max_descent) * font_scale;
    }

    void resize_and_reposition() override {
        float width = get_text_width();
        float height = get_text_max_height();
        set_size(vec3(width,height,1.f));
        recalculate_ui_position();
    }

    // override the transform calculation to NOT set the scale (it is handled by the texture already)
    void calculate_local_transform() override {
        local_transform_matrix = mat4(1.0f);
        local_transform_matrix = glm::translate(local_transform_matrix, this->position);
        local_transform_matrix = glm::rotate(local_transform_matrix, glm::radians(this->rotation.x), V3_X);
        local_transform_matrix = glm::rotate(local_transform_matrix, glm::radians(this->rotation.y), V3_Y);
        local_transform_matrix = glm::rotate(local_transform_matrix, glm::radians(this->rotation.z), V3_Z);
        local_transform_matrix = glm::scale(local_transform_matrix, vec3(1.f));
    }

    void place_on_curve(Curve2D *curve) {
        on_curve = true;
        this->curve = curve;
    }
};

std::map<GLchar, Character> UIText::Characters;
bool UIText::isFontLoaded = false;

#endif