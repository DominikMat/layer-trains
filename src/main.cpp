#include <iostream>
#include "rendering/Window.h"

InputHandler* InputHandler::instance = nullptr; 

void interact_callback () {
    std::cout << "Interaction logged!" << std::endl;
};

int main() {
    // ==========================================================
    /* Initialize glfw and window */

    Window window( SCR_WIDTH, SCR_HEIGHT, WINDOW_TITLE );
    InputHandler input_handler;
    window.initGLFW();
    if (!window.create()) return -1;
    glfwSetScrollCallback(window.get(), InputHandler::scroll_callback);
    double last_frame_time = 0.0f;

    // ==========================================================
    /* Create simulation objects */
    
    Camera world_camera = Camera(SCR_WIDTH, SCR_HEIGHT, 3.f, CAMERA_FOV, CAMERA_NEAR_CLIP_PLANE, CAMERA_FAR_CLIP_PLANE * 1000);
    World world = World(&world_camera);
    InteractableManager intearactable_manager = InteractableManager(&world);
    Terrain terrain = Terrain(&terrain_transalpine, &intearactable_manager, &world_camera);
    Plane *terrain_obj = terrain.get_obj();
    world.place(terrain_obj);
    
    Shader line_shader = ShaderManager::get_line_shader(vec3(0.f, 0.2f, 1.f));
    Line line = Line(LINE_THICKNESS);
    world_camera.set_orthographic(&line_shader);
    line.set_colour( CONTOUR_LINE_COLOUR );
    line.set_parent(terrain_obj);
    line.move(CONTOUR_LINE_HEGHT_OFFSET);
    world.place(&line);

    // --- Interaction Objects ---
    Interactable test_interact = Interactable(vec3(0.f), INTERACTABLE_INTERACT_DISTANCE); // Position 0, will be moved by attach
    test_interact.add_callback(interact_callback);
    terrain.attach_to_surface( &test_interact, 0.5f, 0.5f ); 
    intearactable_manager.add(&test_interact);

    // --- Drawing State ---
    bool is_drawing_path = false;
    vec3 path_start_pos_local;

    // ==========================================================
    /* Create ui */

    ScreenUI screen_ui = ScreenUI();
    Panel test_panel = Panel(Colour::DARK_GREY, vec2(0), vec2(800, 85));
    test_panel.set_anchor( UIAnchor::TOP_LEFT, vec2(10,-10) );
    screen_ui.place( &test_panel );

    Text test_text = Text("Layer Trains Prototype ;)", 1.5f, Colour::WHITE);
    test_text.set_parent(&test_panel);
    test_text.set_anchor( UIAnchor::BOTTOM_LEFT, vec2(15) );
    screen_ui.place( &test_text );
    
    // ==========================================================
    /* Render Loop */

    while(window.open()) {
        // ==========================================================
        /* Prepare program frame - clock and user input */

        float current_time = (float) glfwGetTime();
        float dt = current_time - last_frame_time;
        last_frame_time = current_time;
        
        input_handler.process_input(window.get());

        // ==========================================================
        /* Parse user input */
        
        vec3 camera_mv = input_handler.get_camera_movement();
        int speedup = input_handler.is_holding_shift() ? CAMERA_SHIFT_SPEED_MULTIPLIER : 1.f;
        
        // Note: Using terrain_obj for rotation as per your code, though typically Camera moves around
        terrain_obj->rotate(V3_Z * (camera_mv.x * CAMERA_PAN_SPEED * dt * speedup)); 
        if (camera_mv.z > 0.f && terrain_obj->rotation.x < 0.f) terrain_obj->rotate(V3_X * (camera_mv.z * CAMERA_PAN_SPEED * dt * speedup)); 
        if (camera_mv.z < 0.f && terrain_obj->rotation.x > -90.f) terrain_obj->rotate(V3_X * (camera_mv.z * CAMERA_PAN_SPEED * dt * speedup)); 
        
        world_camera.set_orthographic_zoom(input_handler.get_scroll_value() * 2.f); 
        
        // ==========================================================
        /* Render to world position buffer texture */
        
        glBindFramebuffer(GL_FRAMEBUFFER, terrain.terrain_shader.worldPosFBO);
            window.clear(RENDER_BACKGROUND_COLOUR);
            terrain.terrain_shader.render_to_world_pos_buffer();
            world.render(true);

        // ==========================================================
        // --- Logic ---

        vec3 mouse_pos_world = input_handler.get_mouse_position_world();
        vec4 local_pos_4 = inverse(terrain_obj->get_transform()) * vec4(mouse_pos_world, 1.f);
        vec3 mouse_pos_local = vec3(local_pos_4);

        if (input_handler.is_left_mouse_clicked()) {
            if (!is_drawing_path) {
                // Click 1: Start Drawing
                // Check if mouse is actually on terrain (simple bounds check)
                if (abs(mouse_pos_local.x) <= 0.5f && abs(mouse_pos_local.y) <= 0.5f) {
                    is_drawing_path = true;
                    path_start_pos_local = mouse_pos_local;
                    
                    terrain.elevation_line_drawer.clear_cache();
                    // Force generate initial segment
                    std::vector<vec3> segment = terrain.elevation_line_drawer.get_active_path_segment(path_start_pos_local, mouse_pos_local, 0.f, true);
                    line.set_points(segment);
                    std::cout << "Path started." << std::endl;
                }
            } else {
                // Click 2: End Drawing
                is_drawing_path = false;
                std::cout << "Path finished." << std::endl;
            }
        }
        if (is_drawing_path) {
            // Update path to mouse
            if (abs(mouse_pos_local.x) <= 0.5f && abs(mouse_pos_local.y) <= 0.5f) {
                std::vector<vec3> segment = terrain.elevation_line_drawer.get_active_path_segment(path_start_pos_local, mouse_pos_local);
                
                // Lift line slightly to avoid Z-fighting (Local Z up)
                for(auto& p : segment) p.z += 0.005f; 
                
                line.set_points(segment);
            }
        }

        intearactable_manager.process_all(mouse_pos_local, input_handler.is_left_mouse_clicked());

        //mouse_to_obj_line.set_points( {test_interact.position, } )

        // ==========================================================
        /* Final render */

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
            window.clear(RENDER_BACKGROUND_COLOUR);
            terrain.terrain_shader.bind_world_pos_buffer();
            terrain.terrain_shader.send_mouse_position( input_handler.get_mouse_position_normalized(),  CURSOR_INNER_RADIUS, CURSOR_OUTER_RADIUS );
            world.render();

        screen_ui.render( SCR_WIDTH, SCR_HEIGHT );
        window.display(); 
    }

    glfwTerminate();
    return 0;
}