

#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class Texture
{
public:
    unsigned int ID;
    unsigned int width = -1, height = -1;
    const char* texturePath;

    // generate from file path
    Texture(const char* texturePath, bool flip_vert = true, bool load_16_bit = false) : texturePath(texturePath)
    {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);

        // invert texture
        stbi_set_flip_vertically_on_load(flip_vert);  
        
        // load and generate the texture
        int width, height, nrColourChannels;

        if (load_16_bit) {
            unsigned short* data = stbi_load_16(texturePath, &width, &height, &nrColourChannels, 1);
            if (!data) { std::cout << "Failed to load 16bit texture" << std::endl; return; }
            this->width = width;
            this->height = height;

            glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_RED, GL_UNSIGNED_SHORT, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(data);

        } else {
            unsigned char *data = stbi_load(texturePath, &width, &height, &nrColourChannels, 0); 
            if (!data) { std::cout << "Failed to load texture" << std::endl; return; }
            this->width = width;
            this->height = height;
            int colour_range = nrColourChannels == 4 ? GL_RGBA : nrColourChannels == 3 ? GL_RGB : GL_RED;
            glTexImage2D(GL_TEXTURE_2D, 0, colour_range, width, height, 0, colour_range, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);    
            stbi_image_free(data);
        }

        // set the texture wrapping/filtering options (on the currently bound texture object)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    // generate from raw data
    Texture(int _width, int _height, const unsigned char* data) : width(_width), height(_height)
    {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);

        // Ustawienie parametrów
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Użyj CLAMP dla map
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // activate the shader
    // ------------------------------------------------------------------------
    void use(int slot=0) 
    { 
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, ID); 
    }

    void set_boundry_condition(int property) {
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, property);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, property);
    }
};
#endif