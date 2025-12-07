#include <iostream>
#include "rendering/Window.h"

InputHandler* InputHandler::instance = nullptr; 

int main() {
    // ==========================================================
    /* Initialize glfw and window */

    Window window( SCR_WIDTH, SCR_HEIGHT, WINDOW_TITLE );
    InputHandler input_handler = InputHandler();
    input_handler.set_scroll_speed(CAMERA_ZOOM_SPEED);
    window.initGLFW();
    if (!window.create()) return -1;
    glfwSetScrollCallback(window.get(), InputHandler::scroll_callback);
    double last_frame_time = 0.0f;

    // ==========================================================
    /* Create simulation objects */
    
    Camera camera = Camera(SCR_WIDTH, SCR_HEIGHT, 3.f, CAMERA_FOV, CAMERA_NEAR_CLIP_PLANE, CAMERA_FAR_CLIP_PLANE * 1000);
    camera.set_min_orthographic_zoom(CAMERA_MIN_ZOOM); camera.set_max_orthographic_zoom(CAMERA_MAX_ZOOM);
    World world = World(&camera);
    ScreenUI screen_ui = ScreenUI();
    World world_ui = World(&camera);
    
    // ==========================================================
    /* Create scenes */

    const int scene_num = 1;
    Scene* scenes[scene_num] = {
        new TerrainScene( terrain_transalpine, &world, &camera, &screen_ui, &world_ui, &input_handler)
    };

    // ==========================================================
    /* Render Loop */

    for (int i=0; i<scene_num; i++){
        Scene* current_scene = scenes[i];
        current_scene->init();
        
        Shader *world_pos_buffer_shader = current_scene->get_world_pos_buffer_shader();

        while(current_scene->active()) {

            /* Frame time controls  */
            float current_time = (float) glfwGetTime();
            float dt = current_time - last_frame_time;
            dt = dt > 0.2f ? .2f : dt; // make sure dt not massive on lag spike
            last_frame_time = current_time;
            
            /* Process user input  */
            input_handler.process_input(window.get(), dt);
            
            /* Update camera position for shaders */
            camera.calculate_transform_matrix();
            camera.update_dependent_shader_view_matrix();

            /* Render to world position buffer texture */            
            if (world_pos_buffer_shader){
                glBindFramebuffer(GL_FRAMEBUFFER, world_pos_buffer_shader->worldPosFBO);
                window.clear(RENDER_BACKGROUND_COLOUR);
                world_pos_buffer_shader->render_to_world_pos_buffer();
                world.render(true);
            }

            /* Current scene logic */
            current_scene->loop(dt);

            /* Final render */
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            window.clear(RENDER_BACKGROUND_COLOUR);
            if(world_pos_buffer_shader) {
                world_pos_buffer_shader->bind_world_pos_buffer();
                world_pos_buffer_shader->send_mouse_position( input_handler.get_mouse_position_normalized(),  CURSOR_INNER_RADIUS, CURSOR_OUTER_RADIUS );
            }
            world.render();
            world_ui.render();
            screen_ui.render( SCR_WIDTH, SCR_HEIGHT );

            window.display(); 

            /* check window closed */
            if (!window.open()) {
                glfwTerminate();
                return 0;
            }
        }
    }

    glfwTerminate();
    return 0;
}