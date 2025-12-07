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


class Camera : public Object{
public:
    float screen_width, screen_height, fov, near_clip, far_clip;
    std::vector<Shader*> dependent_shaders;
    std::vector<ProjectionType> dependent_shaders_perspective_type; // each shader can be ortho or perspective
    float orthographic_zoom = 2.f;
    float max_orthographic_zoom = 0.01f;
    float min_orthographic_zoom = 10.f;

    Camera (float screen_width, float screen_height,
        float camera_offset_z = 3.f, float fov = 45.f, float near_clip = 0.1f, float far_clip=100.f) 
        : Object(vec3(0.f,0.f,-camera_offset_z), vec3(1.f)),
        screen_width(screen_width), screen_height(screen_height),
        fov(fov), near_clip(near_clip), 
        far_clip(far_clip) {}

    void set_perspective(Shader *shader, bool add_dependancy = true) { 
        shader->use();
        
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(fov), screen_width/screen_height, near_clip, far_clip);
        shader->setMatrix("projection", projection);
        
        set_view_matrix(shader);
        
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

        set_view_matrix(shader);

        if (add_dependancy) {
            dependent_shaders.push_back(shader);
            dependent_shaders_perspective_type.push_back(ProjectionType::Orthographic);
        }
    }

    void set_view_matrix (Shader *shader) {
        shader->setMatrix("view", get_transform());
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

        set_view_matrix(shader);

        if (add_dependancy) {
            dependent_shaders.push_back(shader);
            dependent_shaders_perspective_type.push_back(ProjectionType::Screenspace);
        }
    }

    /* Zoom */
    void set_orthographic_zoom (float new_zoom) {
        orthographic_zoom = glm::clamp(new_zoom, min_orthographic_zoom, max_orthographic_zoom);
        update_dependent_shader_projection(ProjectionType::Orthographic);
    }
    void change_orthographic_zoom (float delta_zoom) { set_orthographic_zoom(orthographic_zoom+delta_zoom); }
    float get_current_orthographic_zoom () { return orthographic_zoom; }
    void set_max_orthographic_zoom(float new_value) { max_orthographic_zoom = new_value; }
    void set_min_orthographic_zoom(float new_value) { min_orthographic_zoom = new_value; }

    /* FOV */
    void set_fov (float new_fov) {
        fov = glm::clamp(new_fov, 0.01f, 180.f);
        update_dependent_shader_projection(ProjectionType::Screenspace);
    }

    /* Screen size */
    void set_screen_size(float width, float height) {
        screen_width = width;
        screen_height = height;
        update_dependent_shader_projection(ProjectionType::Screenspace);
    }

    /* View matrix  */
    void update_dependent_shader_view_matrix () {
        for (int i=0; i<dependent_shaders.size(); i++) {
            set_view_matrix(dependent_shaders[i]);
        }
    }

    /* Other */
    void set_wireframe() { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  }

    void render() override { std::cout << "Camera compoenent does not have a body so render() is redundant" << std::endl; }
    void construct() override { std::cout << "Camera compoenent does not have a body so construct() is redundant" << std::endl; }

private:
    void update_dependent_shader_projection (ProjectionType projection_type) {
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
