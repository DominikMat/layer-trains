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
    
    // ==========================================================
    /* Create scenes */

    const vector<Scene*> scenes = {
        new TitleCardScene( &world, &camera, &screen_ui, &input_handler),
        new TerrainScene( &terrain_transalpine, &world, &camera, &screen_ui, &input_handler)
    };

    // ==========================================================
    /* Render Loop */

    for (int i=0; i<scenes.size(); i++){
        // remove previous scene objects
        screen_ui.clear_objects();
        world.clear_objects();
        camera.clear_dependancies();

        // init current scene
        std::cout << std::endl << std::endl << "=============================" << std::endl 
            << "Starting scene nr " << (i+1) << std::endl << "=============================" << std::endl;
        Scene* current_scene = scenes[i];
        current_scene->init();
        Shader *world_pos_buffer_shader = current_scene->get_world_pos_buffer_shader();
        vec4 bg_col = current_scene->get_background_colour();
        
        while(current_scene->active()) {
            /* Frame time controls  */
            float current_time = (float) glfwGetTime();
            float dt = current_time - last_frame_time;
            dt = dt > 0.2f ? .2f : dt; // make sure dt not massive on lag spike
            last_frame_time = current_time;
            
            /* Process user input  */
            input_handler.process_input(window.get(), dt, window.get_size());
            screen_ui.check_button_clicked(&input_handler);
            
            /* Update camera position for shaders */
            camera.calculate_transform_matrix();
            camera.update_dependent_shader_view_matrix();

            /* Render to world position buffer texture */            
            if (world_pos_buffer_shader){
                glBindFramebuffer(GL_FRAMEBUFFER, world_pos_buffer_shader->worldPosFBO);
                window.clear(bg_col.r,bg_col.b,bg_col.g,bg_col.a);
                world_pos_buffer_shader->render_to_world_pos_buffer();
                world.render(true);
            }

            /* Current scene logic */
            current_scene->loop(dt);

            /* Final render */
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            window.clear(bg_col.r,bg_col.b,bg_col.g,bg_col.a);
            if(world_pos_buffer_shader) {
                world_pos_buffer_shader->bind_world_pos_buffer();
                world_pos_buffer_shader->send_mouse_position( input_handler.get_mouse_position_normalized(),  CURSOR_INNER_RADIUS, CURSOR_OUTER_RADIUS );
            }
            world.render();
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