#ifndef TEXT_H
#define TEXT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include <iostream>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "UIObject.h"
#include "Shader.h"

struct Character {
    unsigned int TextureID;  
    glm::ivec2   Size;       
    glm::ivec2   Bearing;    
    unsigned int Advance;    
};

class Text : public UIObject {
private:
    std::string textString;
    float font_scale; // Renamed to avoid confusion with Object::size (transform scale)
    unsigned int VAO, VBO;
    bool center_text; // Option to center text around the pivot
    
    static std::map<GLchar, Character> Characters;
    static bool isFontLoaded;

public:
    // Added centered bool. 
    Text(std::string text, float font_scale = 1.0f, vec4 color = Colour::BLACK, bool centered = false)
        : UIObject(vec2(0), vec2(1)), textString(text), font_scale(font_scale), center_text(centered)
    {
        set_colour(color);
        this->uses_texture = true; 
        
        if (!isFontLoaded) {
            loadFont(DEFAULT_FONT); 
        }
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
        // Standard Object Configuration
        Object::configure_render_properties(); 

        glActiveTexture(GL_TEXTURE0);
        shader->setBool("isText", true);
    }

    void render() override {
        if (!visible) return;

        glBindVertexArray(VAO);

        float x = 0.0f;
        float y = 0.0f; 
        
        if (center_text) {
            float width = get_text_width();
            x -= width / 2.0f;
        }

        std::string::const_iterator c;
        for (c = textString.begin(); c != textString.end(); c++) 
        {
            Character ch = Characters[*c];

            float xpos = x + ch.Bearing.x * font_scale;
            float ypos = y - (ch.Size.y - ch.Bearing.y) * font_scale;

            float w = ch.Size.x * font_scale;
            float h = ch.Size.y * font_scale;

            // Z is kept at 0.0f because the Object transform handles 3D placement
            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },            
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },

                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }           
            };

            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            x += (ch.Advance >> 6) * font_scale; 
        }
        
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void disable_render_properties() override {
        shader->setBool("isText", false);   
    }

    void set_text(std::string newText) {
        textString = newText;
    }

    float get_text_width() {
        float width = 0;
        for (char c : textString) {
            width += (Characters[c].Advance >> 6) * font_scale;
        }
        return width;
    }
};

std::map<GLchar, Character> Text::Characters;
bool Text::isFontLoaded = false;

#endif