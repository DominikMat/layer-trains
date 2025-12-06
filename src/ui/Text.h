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

// Struktura przechowująca dane o jednym znaku
struct Character {
    unsigned int TextureID;  // ID tekstury OpenGL
    glm::ivec2   Size;       // Rozmiar glyphu (szerokość, wysokość)
    glm::ivec2   Bearing;    // Przesunięcie od linii bazowej do lewego-górnego rogu glyphu
    unsigned int Advance;    // O ile pikseli przesunąć kursor w prawo (jednostka 1/64 piksela)
};

class Text : public UIObject {
private:
    std::string textString;
    float scale;
    unsigned int VAO, VBO;
    
    // Statyczna mapa znaków - ładujemy czcionkę raz dla wszystkich obiektów Text
    static std::map<GLchar, Character> Characters;
    static bool isFontLoaded;

public:
    Text(std::string text, float scale = 1.0f, vec4 color = Colour::BLACK, vec2 pos = vec2(0))
        : UIObject(pos, vec2(1)), textString(text), scale(scale) // size jest tymczasowy
    {
        this->colour = color;
        this->uses_texture = true; // Tekst technicznie używa tekstury
        
        // Jeśli czcionka nie jest załadowana, załaduj ją (np. Arial lub inna)
        if (!isFontLoaded) {
            loadFont(DEFAULT_FONT); // Ścieżka do czcionki w Windows
        }
    }

    // Ładowanie FreeType (statyczna metoda)
    static void loadFont(const char* fontPath) {
        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return;
        }

        FT_Face face;
        if (FT_New_Face(ft, fontPath, 0, &face)) {
            std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
            return;
        }

        // Ustawienie rozmiaru czcionki (0, 48) -> wysokość 48 pikseli
        FT_Set_Pixel_Sizes(face, 0, 48);

        // Wyłączamy wyrównanie bajtów (tekstury są 1-bajtowe red, a OpenGL lubi 4-bajtowe)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

        // Generujemy tekstury dla znaków ASCII 0-128
        for (unsigned char c = 0; c < 128; c++)
        {
            // Załaduj glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }

            // Generuj teksturę
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            
            // Ważne: Format GL_RED (1 kanał)
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );

            // Opcje tekstury (CLAMP_TO_EDGE eliminuje artefakty na brzegach liter)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // Zapisz znak w mapie
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                (unsigned int)face->glyph->advance.x
            };
            Characters.insert(std::pair<GLchar, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind
        
        // Wyczyść FreeType
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        
        isFontLoaded = true;
        std::cout << "SUCCESS: FreeType Font loaded." << std::endl;
    }

    void construct() override {
        // Konfiguracja VAO/VBO dla dynamicznego quada
        // Używamy GL_DYNAMIC_DRAW, bo będziemy zmieniać wierzchołki dla każdej litery
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        
        // Rezerwujemy pamięć na 6 wierzchołków * 4 floaty (x, y, u, v)
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void configure_render_properties(Shader *s) override {
        glActiveTexture(GL_TEXTURE0);
        
        // Ustawiamy flagę isText na true
        s->setBool("isText", true);
        s->setVec4("colour", colour);
        // Ważne: dla Tekstu zazwyczaj NIE używamy macierzy transformacji obiektu (model),
        // ponieważ każda litera ma własną pozycję obliczaną dynamicznie.
        // Zamiast tego wysyłamy Identity matrix, a pozycje liczymy w CPU.
        // ALE: Twoja kamera ustawia Projection. Musimy tylko upewnić się, że model = Identity
        // lub w vertex shaderze projection * vec4(pos, 0, 1).
        
        // Hack: Resetujemy macierz modelu na Identity, bo pozycje wierzchołków
        // będziemy obliczać w pikselach ekranu bezpośrednio w VBO.
        s->setMatrix("transform", glm::mat4(1.0f));
    }
    void render() override {
        if (!visible) return;

        glBindVertexArray(VAO);

        // Pozycja startowa (z klasy bazowej UIObject)
        // Zakładamy, że UIObject::position to lewy dolny róg tekstu (baseline)
        float x = position.x;
        float y = position.y; 

        // Iteruj po znakach stringa
        std::string::const_iterator c;
        for (c = textString.begin(); c != textString.end(); c++) 
        {
            Character ch = Characters[*c];

            float xpos = x + ch.Bearing.x * scale;
            // Przesunięcie w dół dla znaków wiszących (jak g, j, y)
            float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

            float w = ch.Size.x * scale;
            float h = ch.Size.y * scale;

            // Zbuduj quad dla bieżącego znaku (współrzędne pikselowe)
            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },            
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },

                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }           
            };

            // Renderuj teksturę glyphu
            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            
            // Zaktualizuj zawartość VBO
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
            
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            // Narysuj quad
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Przesuń kursor (advance jest w 1/64 piksela)
            x += (ch.Advance >> 6) * scale; 
        }
        
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    // Metoda do zmiany tekstu w trakcie gry
    void set_text(std::string newText) {
        textString = newText;
    }

    // Metoda pomocnicza do obliczania szerokości tekstu (dla centrowania)
    float get_text_width() {
        float width = 0;
        for (char c : textString) {
            width += (Characters[c].Advance >> 6) * scale;
        }
        return width;
    }
};

// Inicjalizacja pól statycznych (w .cpp lub na końcu headera jeśli inline)
std::map<GLchar, Character> Text::Characters;
bool Text::isFontLoaded = false;

#endif