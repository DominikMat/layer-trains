#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "textures/Texture.h"
#include "Settings.h"

// Zakładam istnienie tych plików, jeśli nie masz, usuń include'y poniżej
// #include "textures/TextureData.h" 
// #include "Heightmap.h"
// #include "settings/Settings.h" 

#define MAX_TEXTURE_SLOTS 16

class Shader
{
public:
    unsigned int ID;
    std::vector<Texture*> textures;
    bool heightmap_enabled;
    bool uses_texture = false;

    // --- Globalne zmienne renderera (zostawiam bez zmian) ---
    GLuint worldPosFBO;          
    GLuint worldPosTexture;      
    GLuint worldPosDepthRBO;    

    // Konstruktor teraz przyjmuje opcjonalny 3. argument
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr) 
        : heightmap_enabled(false)
    {
        textures.clear();
        
        // 1. Retrieve the vertex/fragment/geometry source code
        std::string vertexCode;
        std::string fragmentCode;
        std::string geometryCode;

        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        std::ifstream gShaderFile;

        // Ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        gShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

        try 
        {
            // Open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            
            // Read buffers
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            
            // Close handlers
            vShaderFile.close();
            fShaderFile.close();
            
            // Convert to string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();

            // --- Geometry Shader Loading ---
            if (geometryPath != nullptr)
            {
                gShaderFile.open(geometryPath);
                std::stringstream gShaderStream;
                gShaderStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gShaderStream.str();
            }
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        // 2. Compile shaders
        unsigned int vertex, fragment, geometry;

        // Vertex Shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");

        // Fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");

        // Geometry Shader (Optional)
        if (geometryPath != nullptr)
        {
            const char* gShaderCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
        }

        // Shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        if (geometryPath != nullptr)
            glAttachShader(ID, geometry);
            
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        // Delete the shaders as they're linked now
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (geometryPath != nullptr)
            glDeleteShader(geometry);
    }

    void use() 
    { 
        glUseProgram(ID); 
        for (int i=0; i<textures.size(); i++) textures[i]->use(i);
    }

    void setBool(const std::string &name, bool value) const
    {          
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
    }
    void setInt(const std::string &name, int value) const
    { 
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }
    void setFloat(const std::string &name, float value) const
    { 
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
    }
    void setVec2(const std::string &name, const glm::vec2 &value) const
    { 
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value)); 
    }
    void setVec3(const std::string &name, const glm::vec3 &value) const
    { 
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value)); 
    }
    void setVec4(const std::string &name, const glm::vec4&value) const
    { 
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value)); 
    }
    void setMatrix(const std::string &name, glm::mat4 matrix){
        unsigned int loc = glGetUniformLocation(ID, name.c_str());
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
    }
    void addTexture(Texture* new_tex){
        if (textures.size() >= MAX_TEXTURE_SLOTS) {
            std::cout<< "Max number of textures loaded reached!" << std::endl;
            return;
        } 
        uses_texture = true;
        textures.push_back(new_tex);
    }
    int get_last_loaded_tex_slot() {
        return textures.size()-1;
    }
    
    void config_worldpos_buffer() {
        // 2. Create the Framebuffer Object (FBO)
        glGenFramebuffers(1, &worldPosFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, worldPosFBO);

        // 3. Create the World Position texture (the FBO's color buffer)
        glGenTextures(1, &worldPosTexture);
        glBindTexture(GL_TEXTURE_2D, worldPosTexture);
        // **IMPORTANT:** Use a high-precision format! GL_RGB32F stores 3 floats (x, y, z).
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // Attach this texture to the FBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, worldPosTexture, 0);

        // 4. Create a Renderbuffer Object (RBO) for depth testing
        glGenRenderbuffers(1, &worldPosDepthRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, worldPosDepthRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, SCR_WIDTH, SCR_HEIGHT);
        // Attach this RBO to the FBO
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, worldPosDepthRBO);

        // 5. Check if the FBO is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        }

        // 6. Unbind the FBO so we render to the main screen by default
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void bind_world_pos_buffer () {
        use();
        setBool("u_renderWorldPos", false);
        glActiveTexture(GL_TEXTURE15);
        glBindTexture(GL_TEXTURE_2D, worldPosTexture);
        setInt("world_pos_texture", 15);
    }
    void render_to_world_pos_buffer() {
        use();
        setBool("u_renderWorldPos", true);
    }
    void send_mouse_position(glm::vec2 mouse_pos, float inner_raduis, float outer_radius) {
        setVec2("u_mouseCoords", mouse_pos);
        setFloat("u_circleInnerRadius", inner_raduis);  
        setFloat("u_circleOuterRadius", outer_radius);
    }

private:
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};
#endif