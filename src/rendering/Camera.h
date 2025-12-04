#ifndef CAMERA_H
#define CAMERA_H

#include "world_objects/Object.h" // The base class for all renderable entities
#include "shaders/Shader.h" // The base class for all renderable entities
#include <vector>
#include <memory>   // Required for std::unique_ptr
#include <glm/glm.hpp>

using namespace glm;

enum class ProjectionType {
    Perspective, Orthographic, Screenspace
};


class Camera {
public:
    vec3 position;
    float screen_width, screen_height, fov, near_clip, far_clip;
    std::vector<Shader*> dependent_shaders;
    std::vector<ProjectionType> dependent_shaders_perspective_type; // each shader can be ortho or perspective
    float orthographic_zoom = 2.f;

    Camera (float screen_width, float screen_height,
        float camera_offset_z = -3.f, float fov = 45.f, float near_clip = 0.1f, float far_clip=100.f) 
        : screen_width(screen_width), screen_height(screen_height),
        fov(fov), position(vec3(0.f,0.f,camera_offset_z)), near_clip(near_clip), 
        far_clip(far_clip) {}

    void set_perspective(Shader *shader, bool add_dependancy = true) { 
        shader->use();
        
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(fov), screen_width/screen_height, near_clip, far_clip);
        shader->setMatrix("projection", projection);
        
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, -position); // camera movement is just moving the scene in opposite dir
        shader->setMatrix("view", view);
        
        if (add_dependancy) {
            dependent_shaders.push_back(shader);
            dependent_shaders_perspective_type.push_back(ProjectionType::Perspective);
        }
    }
    
    void set_orthographic(Shader *shader, bool add_dependancy = true) {
        shader->use();

        // 1. Obliczamy granice projekcji, wyśrodkowane wokół (0, 0)
        float aspect_ratio = screen_width / screen_height;
        
        // Używamy orthographic_zoom do określenia jednostek świata, które mają być widoczne
        float ortho_width = orthographic_zoom * aspect_ratio;
        float ortho_height = orthographic_zoom;

        glm::mat4 projection;
        // Ustawiamy symetryczne granice: od -W/2 do +W/2, od -H/2 do +H/2
        projection = glm::ortho(
            -ortho_width / 2.0f, ortho_width / 2.0f, 
            -ortho_height / 2.0f, ortho_height / 2.0f, 
            near_clip, far_clip);
            
        shader->setMatrix("projection", projection);

        // 2. Macierz widoku (translacja kamery)
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, -position); 
        shader->setMatrix("view", view);

        if (add_dependancy) {
            dependent_shaders.push_back(shader);
            dependent_shaders_perspective_type.push_back(ProjectionType::Orthographic);
        }
    }
    
    void set_screenspace(Shader *shader, bool add_dependancy = true) {
        shader->use();

        glm::mat4 projection;
        projection = glm::ortho(
            0.f, screen_width,
            0.f, screen_height,
            near_clip, far_clip
        );
            
        shader->setMatrix("projection", projection);

        // 2. Macierz widoku (translacja kamery)
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, -position); 
        shader->setMatrix("view", view);

        if (add_dependancy) {
            dependent_shaders.push_back(shader);
            dependent_shaders_perspective_type.push_back(ProjectionType::Screenspace);
        }
    }

    void set_wireframe() {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    void mv(vec3 mv) {
        position += mv;
    }

    void set_orthographic_zoom (float zoom) {
        orthographic_zoom = glm::clamp(zoom, 0.01f, 1000.f);
        update_dependent_shaders(ProjectionType::Orthographic);
    }
    void set_fov (float new_fov) {
        fov = glm::clamp(new_fov, 0.01f, 180.f);
        update_dependent_shaders(ProjectionType::Screenspace);
    }

    void set_screen_size(float width, float height) {
        screen_width = width;
        screen_height = height;
        update_dependent_shaders(ProjectionType::Screenspace);
    }

private:
    void update_dependent_shaders (ProjectionType projection_type) {
        for (int i=0; i<dependent_shaders.size(); i++) {
            if (dependent_shaders_perspective_type[i] == projection_type) {
                switch (projection_type) {
                    case ProjectionType::Perspective: set_perspective(dependent_shaders[i], false); break;
                    case ProjectionType::Orthographic: set_orthographic(dependent_shaders[i], false); break;
                    case ProjectionType::Screenspace: set_screenspace(dependent_shaders[i], false); break;
                }
            }
        }
    }
};

#endif // CAMERA_H
