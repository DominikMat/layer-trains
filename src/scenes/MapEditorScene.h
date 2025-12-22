#ifndef MAP_EDITOR_SCENE_H
#define MAP_EDITOR_SCENE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <random>

// Project Dependencies
#include "Scene.h"
#include "TerrainData.h"
#include "Texture.h" 
#include "textures/TextureData.h" // For gradient paths

// External Libs
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "json.hpp"

// Image writing
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#endif

// Image loading
#ifndef STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" 
#endif

namespace fs = std::filesystem;

enum class BrushShape {
    CIRCLE,
    OVAL_H,
    OVAL_V,
    NOISE_DOTS,
    NOISE_LINES
};

struct MapTile {
    std::string filename;
    bool loaded = false;
};

class MapEditorScene : public Scene
{
    using json = nlohmann::json;

private:
    GLFWwindow* window = nullptr;   

    // Constants
    const std::string BASE_PATH = "C:/Media/Projects/OpenGL/Layer_Trains/user-created/";
    const std::string LEVELS_PATH = BASE_PATH + "levels/";
    const std::string HEIGHTMAPS_PATH = BASE_PATH + "heightmaps/";

    // Data being edited
    TerrainData edit_data;

    // --- PAINTER STATE ---
    std::vector<unsigned char> heightmap_pixels; // Raw 8-bit grayscale
    std::vector<unsigned char> colourmap_pixels; // RGB Preview
    GLuint heightmap_tex_id = 0;
    GLuint colourmap_tex_id = 0;
    
    int hmap_width = 1024;
    int hmap_height = 1024;
    
    // Brush
    float brush_size = 50.0f;
    float brush_strength = 5.0f;
    BrushShape current_brush = BrushShape::CIRCLE;
    
    // View
    bool show_colour_preview = false;
    bool show_tags_preview = true;

    // Gradients for Color generation (Loaded to RAM for CPU processing)
    std::vector<glm::vec3> grad_elev;
    std::vector<glm::vec3> grad_steep;
    std::vector<glm::vec3> grad_water;

    // --- TILEMAP STATE ---
    static const int GRID_ROWS = 3;
    static const int GRID_COLS = 3;
    MapTile tile_grid[GRID_ROWS][GRID_COLS];
    int current_tile_x = 1;
    int current_tile_y = 1;

    // --- GUI STATE ---
    std::string title_buf = "";
    std::string tag_names[MAX_TAG_AMOUNT];
    std::string external_import_path = ""; // Buffer for external import
    
    // Import/Save
    std::vector<std::string> available_levels;
    int selected_level_idx = -1;
    
    // Feedback
    std::string status_msg = "";
    float status_timer = 0.0f;
    bool status_is_error = false;

    /* other */
    bool initalized = false;

public:

    MapEditorScene (GLFWwindow *win, World *w, Camera *c, ScreenUI *s, InputHandler *ih) 
        : Scene(w,c,s,ih), window(win) 
    {
        // 1. Ensure directories exist
        if (!fs::exists(LEVELS_PATH)) fs::create_directories(LEVELS_PATH);
        if (!fs::exists(HEIGHTMAPS_PATH)) fs::create_directories(HEIGHTMAPS_PATH);

        // 2. Load Gradients into memory for fast CPU preview generation
        load_gradients_to_ram();

        // 3. Load Default Data
        edit_data = terrain_transalpine; 
        load_heightmap_to_memory(edit_data.heightmap_path);
        
        sync_buffers_from_struct();
        refresh_level_list();

        // Init Tilemap names
        for(int y=0; y<GRID_ROWS; y++) 
            for(int x=0; x<GRID_COLS; x++) 
                tile_grid[y][x].filename = "tile_" + std::to_string(x) + "_" + std::to_string(y);
    }
    
    void init( ) override {
        if (initalized) return;
        initalized = true;
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    void loop(float dt) override {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        if (w == 0 || h == 0) return;

        if (status_timer > 0.0f) status_timer -= dt;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- LAYOUT CALCULATION ---
        float left_w = w * 0.2f;
        float right_w = w * 0.2f;
        float center_w = w * 0.6f;
        float full_h = (float)h;

        // Draw Panels
        draw_left_panel(0, 0, left_w, full_h);
        draw_center_panel(left_w, 0, center_w, full_h);
        draw_right_panel(left_w + center_w, 0, right_w, full_h);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    ~MapEditorScene() {
        if (heightmap_tex_id != 0) glDeleteTextures(1, &heightmap_tex_id);
        if (colourmap_tex_id != 0) glDeleteTextures(1, &colourmap_tex_id);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

private:

    // =========================================================
    // PANEL 1: TOOLS (LEFT)
    // =========================================================
    void draw_left_panel(float x, float y, float w, float h) {
        ImGui::SetNextWindowPos(ImVec2(x, y));
        ImGui::SetNextWindowSize(ImVec2(w, h));
        
        ImGui::Begin("Tools", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
        
        ImGui::TextDisabled("MAP EDITOR TOOLKIT");
        ImGui::Separator();

        // --- VIEW SETTINGS ---
        ImGui::Text("View Mode");
        if (ImGui::Button("Heightmap", ImVec2(w * 0.45f, 0))) show_colour_preview = false;
        ImGui::SameLine();
        if (ImGui::Button("Colormap", ImVec2(w * 0.45f, 0))) {
            generate_colour_map_preview();
            show_colour_preview = true;
        }
        
        // RECOLOUR BUTTON
        if (ImGui::Button("Force Recolour", ImVec2(-1, 0))) {
            generate_colour_map_preview();
            show_colour_preview = true; // Automatically switch to color view
        }

        ImGui::Checkbox("Show Tag Markers", &show_tags_preview);
        
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::Separator();

        // --- BRUSH SETTINGS ---
        ImGui::Text("Brush Settings");
        ImGui::SliderFloat("Size", &brush_size, 1.0f, 200.0f);
        ImGui::SliderFloat("Strength", &brush_strength, 0.1f, 20.0f);
        
        ImGui::Text("Shape:");
        if (ImGui::RadioButton("Circle", current_brush == BrushShape::CIRCLE)) current_brush = BrushShape::CIRCLE;
        if (ImGui::RadioButton("Oval (H)", current_brush == BrushShape::OVAL_H)) current_brush = BrushShape::OVAL_H;
        if (ImGui::RadioButton("Oval (V)", current_brush == BrushShape::OVAL_V)) current_brush = BrushShape::OVAL_V;
        if (ImGui::RadioButton("Noise (Dots)", current_brush == BrushShape::NOISE_DOTS)) current_brush = BrushShape::NOISE_DOTS;
        if (ImGui::RadioButton("Noise (Lines)", current_brush == BrushShape::NOISE_LINES)) current_brush = BrushShape::NOISE_LINES;

        ImGui::Dummy(ImVec2(0, 10));
        ImGui::Separator();

        // --- TILEMAP SYSTEM ---
        ImGui::Text("Tilemap Manager");
        
        // Draw 3x3 Grid
        float grid_size = w - 30; // Padding
        float slot_size = grid_size / 3.0f;
        
        for (int row = 0; row < GRID_ROWS; row++) {
            for (int col = 0; col < GRID_COLS; col++) {
                ImGui::PushID(row * GRID_COLS + col);
                
                bool is_active = (row == current_tile_y && col == current_tile_x);
                if (is_active) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
                
                std::string lbl = std::to_string(col) + "," + std::to_string(row);
                if (ImGui::Button(lbl.c_str(), ImVec2(slot_size - 5, slot_size - 5))) {
                    // Switch tile logic here (save current, load new)
                    current_tile_x = col;
                    current_tile_y = row;
                    // In real app: save_current(); load_tile(tile_grid[row][col].filename);
                }
                
                if (is_active) ImGui::PopStyleColor();
                
                if (col < GRID_COLS - 1) ImGui::SameLine();
                ImGui::PopID();
            }
        }
        ImGui::TextDisabled("Current Tile: %d, %d", current_tile_x, current_tile_y);

        ImGui::Separator();
        ImGui::Separator();

        if (ImGui::Button("Return to main menu", ImVec2(w * 0.8f, 0))) {
            end_scene(SceneID::TITLE_CARD);
        }

        ImGui::End();
    }

    // =========================================================
    // PANEL 2: PREVIEW (CENTER)
    // =========================================================
    void draw_center_panel(float x, float y, float w, float h) {
        ImGui::SetNextWindowPos(ImVec2(x, y));
        ImGui::SetNextWindowSize(ImVec2(w, h));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0)); // No padding for full view
        
        ImGui::Begin("Preview", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
        
        // Decide texture to show
        GLuint tex_id = show_colour_preview ? colourmap_tex_id : heightmap_tex_id;
        
        // Calculate Aspect Ratio to fit in center window
        ImVec2 content_size = ImGui::GetContentRegionAvail();
        float aspect = (float)hmap_width / (float)hmap_height;
        float draw_w = content_size.x;
        float draw_h = draw_w / aspect;
        
        // If height is too big, scale by height
        if (draw_h > content_size.y) {
            draw_h = content_size.y;
            draw_w = draw_h * aspect;
        }

        // Center the image
        float offset_x = (content_size.x - draw_w) * 0.5f;
        float offset_y = (content_size.y - draw_h) * 0.5f;
        ImGui::SetCursorPos(ImVec2(offset_x, offset_y));

        ImVec2 img_start = ImGui::GetCursorScreenPos();
        ImGui::Image((void*)(intptr_t)tex_id, ImVec2(draw_w, draw_h));

        // --- INTERACTION ---
        if (ImGui::IsItemHovered()) {
            ImVec2 mouse_pos = ImGui::GetMousePos();
            float u = (mouse_pos.x - img_start.x) / draw_w;
            float v = (mouse_pos.y - img_start.y) / draw_h;

            // Draw Brush Preview
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            float preview_radius = (brush_size / (float)hmap_width) * draw_w; 
            
            if (current_brush == BrushShape::OVAL_H) 
                draw_list->AddEllipse(mouse_pos, ImVec2(preview_radius * 2.0f, preview_radius * 0.5f), IM_COL32(255, 255, 0, 200));
            else if (current_brush == BrushShape::OVAL_V) 
                draw_list->AddEllipse(mouse_pos, ImVec2(preview_radius * 0.5f, preview_radius * 2.0f), IM_COL32(255, 255, 0, 200));
            else 
                draw_list->AddCircle(mouse_pos, preview_radius, IM_COL32(255, 255, 0, 200));

            // Apply Paint
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) apply_brush(u, v, true);
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) apply_brush(u, v, false);
        }

        // --- TAG PREVIEW ---
        if (show_tags_preview) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            for(int i=0; i<MAX_TAG_AMOUNT; i++) {
                if (edit_data.tags[i].type == TerrainTagType::DISABLED) continue;
                
                float tx = img_start.x + edit_data.tags[i].uv_x * draw_w;
                float ty = img_start.y + (1.f-edit_data.tags[i].uv_y) * draw_h;
                
                // Draw Dot
                draw_list->AddCircleFilled(ImVec2(tx, ty), 5.0f, IM_COL32(255, 0, 0, 255));
                draw_list->AddCircle(ImVec2(tx, ty), 6.0f, IM_COL32(255, 255, 255, 255));
                
                // Draw Text Label
                std::string lbl = !edit_data.tags[i].name.empty() ? edit_data.tags[i].name : "Tag";
                draw_list->AddText(ImVec2(tx + 8, ty - 10), IM_COL32(255, 255, 255, 255), lbl.c_str());
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    // =========================================================
    // PANEL 3: SETTINGS (RIGHT)
    // =========================================================
    void draw_right_panel(float x, float y, float w, float h) {
        ImGui::SetNextWindowPos(ImVec2(x, y));
        ImGui::SetNextWindowSize(ImVec2(w, h));
        
        ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

        // CREATE NEW
        if (ImGui::Button("Create New Map", ImVec2(-1, 30))) {
            create_new_map();
        }
        
        ImGui::Separator();

        // IMPORT EXTERNAL
        ImGui::Text("Import Heightmap (PNG)");
        char path_buf[256];
        strncpy_s(path_buf, external_import_path.c_str(), 255);
        if (ImGui::InputText("##Path", path_buf, 256)) {
            external_import_path = path_buf;
        }
        if (ImGui::Button("Import from Path", ImVec2(-1, 0))) {
            load_heightmap_to_memory(external_import_path.c_str());
            status_msg = "Imported External PNG!";
            status_timer = 2.0f;
            status_is_error = false;
        }

        ImGui::Separator();

        // SAVE
        if (ImGui::Button("Save JSON & PNG", ImVec2(-1, 30))) save_level();
        
        ImGui::Separator();
        ImGui::Text("Edit Existing");
        ImGui::PushItemWidth(-1);
        
        const char* preview = (selected_level_idx >= 0 && selected_level_idx < available_levels.size()) 
                              ? available_levels[selected_level_idx].c_str() : "Select Level...";
        
        if (ImGui::BeginCombo("##import", preview)) {
            for (int i = 0; i < available_levels.size(); i++) {
                bool is_selected = (selected_level_idx == i);
                if (ImGui::Selectable(available_levels[i].c_str(), is_selected)) {
                    selected_level_idx = i;
                    load_level(available_levels[i]);
                }
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();

        if (status_timer > 0.0f) {
            ImGui::TextColored(status_is_error ? ImVec4(1,0,0,1) : ImVec4(0,1,0,1), "%s", status_msg.c_str());
        }

        ImGui::Separator();
        ImGui::Text("Metadata");

        char path_buf_2[sizeof(title_buf)];
        strncpy_s(path_buf_2, title_buf.c_str(), sizeof(title_buf)-1);
        if (ImGui::InputText("Title", path_buf_2, sizeof(title_buf))) {
            title_buf = path_buf_2;
        }

        ImGui::TextDisabled("File: %s", LEVELS_PATH.c_str());

        ImGui::Separator();
        if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::InputInt("Res X", &edit_data.resolution_x);
            ImGui::InputInt("Res Y", &edit_data.resolution_y);
            ImGui::DragFloat("Min H", &edit_data.minimum_height_reach);
            ImGui::DragFloat("Max H", &edit_data.maximum_height_reach);
            ImGui::InputFloat("Scale", &edit_data.vertical_scale, 0.0001f);
        }

        if (ImGui::CollapsingHeader("Biome", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat("Water", &edit_data.water_level_height, edit_data.minimum_height_reach, edit_data.maximum_height_reach);
            ImGui::SliderFloat("Snow", &edit_data.snow_level_height, edit_data.minimum_height_reach, edit_data.maximum_height_reach);
        }

        if (ImGui::CollapsingHeader("Tags")) {
            for (int i = 0; i < MAX_TAG_AMOUNT; i++) {
                if (edit_data.tags[i].type == TerrainTagType::DISABLED) continue;
                ImGui::PushID(i);
                
                if (ImGui::Button("X")) {
                    edit_data.tags[i].type = TerrainTagType::DISABLED;
                    tag_names[i][0] = '\0';
                }
                ImGui::SameLine();
                if (ImGui::TreeNode(tag_names[i][0] == '\0' ? "Tag" : tag_names[i].c_str())) {
                    // Type
                    int type = (int)edit_data.tags[i].type;
                    const char* types[] = { "Name", "Start", "End", "Disabled" };
                    ImGui::Combo("Type", &type, types, 4);
                    edit_data.tags[i].type = (TerrainTagType)type;
                    
                    char path_buf_3[64];
                    strncpy_s(path_buf_3, tag_names[i].c_str(), 63);
                    if (ImGui::InputText("Lbl", path_buf_3, 64)){
                        tag_names[i] = path_buf_3;
                    }

                    ImGui::SliderFloat("U", &edit_data.tags[i].uv_x, 0.0f, 1.0f);
                    ImGui::SliderFloat("V", &edit_data.tags[i].uv_y, 0.0f, 1.0f);
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            if (ImGui::Button("Add Tag")) {
                for(int i=0; i<MAX_TAG_AMOUNT; i++) {
                    if(edit_data.tags[i].type == TerrainTagType::DISABLED) {
                        edit_data.tags[i].type = TerrainTagType::NAME_TAG;
                        edit_data.tags[i].uv_x = 0.5f; edit_data.tags[i].uv_y = 0.5f;
                        tag_names[i] = "New Tag";
                        break;
                    }
                }
            }
        }

        ImGui::End();
    }

    // =========================================================
    // BRUSH LOGIC
    // =========================================================

    void apply_brush(float u, float v, bool raise) {
        if (heightmap_pixels.empty()) return;

        int cx = (int)(u * hmap_width);
        int cy = (int)(v * hmap_height);
        float radius = brush_size;
        
        // Bounds check optimized for loop range
        int start_y = std::max(0, (int)(cy - radius * 2));
        int end_y = std::min(hmap_height, (int)(cy + radius * 2));
        int start_x = std::max(0, (int)(cx - radius * 2));
        int end_x = std::min(hmap_width, (int)(cx + radius * 2));

        bool changed = false;

        for (int y = start_y; y < end_y; y++) {
            for (int x = start_x; x < end_x; x++) {
                
                float dx = (float)(x - cx);
                float dy = (float)(y - cy);
                float dist_sq = dx*dx + dy*dy;
                float falloff = 0.0f;

                // --- SHAPE LOGIC ---
                switch (current_brush) {
                    case BrushShape::CIRCLE:
                        if (dist_sq > radius * radius) continue;
                        falloff = 1.0f - (sqrt(dist_sq) / radius);
                        break;
                    
                    case BrushShape::OVAL_H: // Horizontal stretch
                        if ((dx*dx)/(radius*radius*4.0f) + (dy*dy)/(radius*radius*0.25f) > 1.0f) continue;
                        falloff = 1.0f - sqrt((dx*dx)/(radius*radius*4.0f) + (dy*dy)/(radius*radius*0.25f));
                        break;

                    case BrushShape::OVAL_V: // Vertical stretch
                        if ((dx*dx)/(radius*radius*0.25f) + (dy*dy)/(radius*radius*4.0f) > 1.0f) continue;
                        falloff = 1.0f - sqrt((dx*dx)/(radius*radius*0.25f) + (dy*dy)/(radius*radius*4.0f));
                        break;

                    case BrushShape::NOISE_DOTS:
                        if (dist_sq > radius * radius) continue;
                        if ((rand() % 100) > 20) continue; // 20% density
                        falloff = 1.0f - (sqrt(dist_sq) / radius);
                        break;

                    case BrushShape::NOISE_LINES:
                        if (dist_sq > radius * radius) continue;
                        // Simple linear noise pattern based on x
                        if ( abs((x + y) % 10) > 2 ) continue; 
                        falloff = 1.0f - (sqrt(dist_sq) / radius);
                        break;
                }

                if (falloff <= 0.0f) continue;
                falloff = falloff * falloff; // Smooth quad curve

                int idx = y * hmap_width + x;
                float val = (float)heightmap_pixels[idx];
                float change = brush_strength * falloff;
                
                val = raise ? (val + change) : (val - change);
                val = std::clamp(val, 0.0f, 255.0f);
                
                if (heightmap_pixels[idx] != (unsigned char)val) {
                    heightmap_pixels[idx] = (unsigned char)val;
                    changed = true;
                }
            }
        }

        if (changed) {
            update_texture(heightmap_tex_id, heightmap_pixels.data(), false);
            // If viewing color, auto-update it roughly (or demand button press for perf)
            if (show_colour_preview) generate_colour_map_preview();
        }
    }

    // =========================================================
    // COLOR MAP GENERATION (CPU)
    // =========================================================

    // Loads gradient textures into vectors for fast CPU lookup
    void load_gradients_to_ram() {
        auto load_to_vec = [](const char* path, std::vector<glm::vec3>& vec) {
            int w, h, ch;
            unsigned char* data = stbi_load(path, &w, &h, &ch, 3);
            if(data) {
                vec.resize(w);
                for(int i=0; i<w; i++) {
                    vec[i] = glm::vec3(data[i*3]/255.f, data[i*3+1]/255.f, data[i*3+2]/255.f);
                }
                stbi_image_free(data);
            }
        };
        load_to_vec(GRADIENT_ELEVATION_PATH, grad_elev);
        load_to_vec(GRADIENT_STEEPNESS_PATH, grad_steep);
        load_to_vec(GRADIENT_WATER_PATH, grad_water);
    }

    glm::vec3 sample_gradient(const std::vector<glm::vec3>& grad, float t) {
        if(grad.empty()) return glm::vec3(1,0,1); // Error pink
        t = std::clamp(t, 0.0f, 1.0f);
        int idx = (int)(t * (grad.size() - 1));
        return grad[idx];
    }

    void generate_colour_map_preview() {
        if (colourmap_pixels.size() != hmap_width * hmap_height * 3)
            colourmap_pixels.resize(hmap_width * hmap_height * 3);

        if (colourmap_tex_id == 0) glGenTextures(1, &colourmap_tex_id);

        for(int y=0; y<hmap_height; y++) {
            for(int x=0; x<hmap_width; x++) {
                int idx_h = y * hmap_width + x;
                int idx_c = idx_h * 3;

                // 1. Raw Height (0-1)
                float h_val = heightmap_pixels[idx_h] / 255.0f;
                
                // 2. Real Elevation
                float elevation = edit_data.minimum_height_reach + h_val * (edit_data.maximum_height_reach - edit_data.minimum_height_reach);

                // 3. Steepness (Simple derivative)
                float h_right = (x < hmap_width-1) ? heightmap_pixels[idx_h+1]/255.f : h_val;
                float h_down  = (y < hmap_height-1) ? heightmap_pixels[idx_h+hmap_width]/255.f : h_val;
                float dx = abs(h_val - h_right);
                float dy = abs(h_val - h_down);
                float steepness = sqrt(dx*dx + dy*dy) * STEEPNESS_SCALE * 10.0f; 

                glm::vec3 final_col;

                if (elevation < edit_data.water_level_height) {
                    // Water
                    float depth = (edit_data.water_level_height - elevation) / 500.0f; 
                    final_col = sample_gradient(grad_water, depth);
                } else {
                    // Land
                    float t_elev = elevation / ELEVATION_GRADIENT_MAX_HEIGHT;
                    glm::vec3 c_elev = sample_gradient(grad_elev, t_elev);
                    glm::vec3 c_steep = sample_gradient(grad_steep, steepness);
                    final_col = glm::mix(c_steep, c_elev, ELEVATION_GRADIENT_STRENGTH); // Mix based on strength
                    
                    // Snow
                    if (elevation > edit_data.snow_level_height) {
                        float snow_f = (elevation - edit_data.snow_level_height) / SNOW_FALLOFF_RANGE;
                        final_col = glm::mix(final_col, glm::vec3(1.0f), std::clamp(snow_f, 0.0f, 1.0f));
                    }
                }

                colourmap_pixels[idx_c + 0] = (unsigned char)(final_col.r * 255);
                colourmap_pixels[idx_c + 1] = (unsigned char)(final_col.g * 255);
                colourmap_pixels[idx_c + 2] = (unsigned char)(final_col.b * 255);
            }
        }
        update_texture(colourmap_tex_id, colourmap_pixels.data(), true);
    }

    // =========================================================
    // CORE HELPERS
    // =========================================================

    void create_new_map() {
        // Reset to default struct
        edit_data = terrain_transalpine; 
        title_buf = "New Map";
        
        // Reset buffers
        hmap_width = 1024;
        hmap_height = 1024;
        heightmap_pixels.assign(hmap_width * hmap_height, 20); // Dark grey flat terrain
        
        // Clear tags
        for(int k=0; k<MAX_TAG_AMOUNT; k++) {
            edit_data.tags[k].type = TerrainTagType::DISABLED;
            tag_names[k][0] = '\0';
        }
        
        sync_buffers_from_struct();
        if (heightmap_tex_id == 0) glGenTextures(1, &heightmap_tex_id);
        update_texture(heightmap_tex_id, heightmap_pixels.data(), false);
        
        status_msg = "New Map Created!";
        status_timer = 2.0f;
    }

    void load_heightmap_to_memory(const std::string& path) {
        int w, h, ch;
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 1);
        if (data) {
            hmap_width = w; hmap_height = h;
            heightmap_pixels.assign(data, data + (w * h));
            stbi_image_free(data);
        } else {
            // Only warn if path was actually provided
            if (!path.empty()) std::cout << "Warning: Could not load heightmap at: " << path << std::endl;
            // Ensure we have something
            if (heightmap_pixels.empty()) {
                hmap_width = 1024; hmap_height = 1024;
                heightmap_pixels.assign(1024*1024, 0);
            }
        }
        if (heightmap_tex_id == 0) glGenTextures(1, &heightmap_tex_id);
        update_texture(heightmap_tex_id, heightmap_pixels.data(), false);
    }

    void update_texture(GLuint id, unsigned char* data, bool rgb) {
        glBindTexture(GL_TEXTURE_2D, id);
        GLenum format = rgb ? GL_RGB : GL_RED;
        GLenum internal = rgb ? GL_RGB : GL_RED;
        // Swizzle for grayscale to appear red/white in preview if needed, or just use GL_RED and let ImGui render red channel
        if (!rgb) { 
            GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        }
        glTexImage2D(GL_TEXTURE_2D, 0, internal, hmap_width, hmap_height, 0, format, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void sync_buffers_from_struct() {
        if(!edit_data.title.empty()) title_buf = edit_data.title;
        for(int i=0; i<MAX_TAG_AMOUNT; i++) {
            if(!edit_data.tags[i].name.empty()) tag_names[i] = edit_data.tags[i].name, 64;
            else tag_names[i][0]='\0';
        }
    }

    void refresh_level_list() {
        available_levels.clear();
        if (fs::exists(LEVELS_PATH)) {
            for (const auto& entry : fs::directory_iterator(LEVELS_PATH)) {
                if (entry.path().extension() == ".json")
                    available_levels.push_back(entry.path().filename().string());
            }
        }
    }

    void save_level() {
        std::string clean = title_buf;
        if(clean.empty()) clean = "untitled";
        std::replace(clean.begin(), clean.end(), ' ', '_');
        
        std::string json_p = LEVELS_PATH + clean + ".json";
        std::string png_p = HEIGHTMAPS_PATH + clean + "_heightmap.png";

        if(stbi_write_png(png_p.c_str(), hmap_width, hmap_height, 1, heightmap_pixels.data(), hmap_width)) {
            json j;
            j["title"] = std::string(title_buf);
            j["heightmap_path"] = png_p;
            j["resolution_x"] = edit_data.resolution_x;
            j["resolution_y"] = edit_data.resolution_y;
            j["min_height"] = edit_data.minimum_height_reach;
            j["max_height"] = edit_data.maximum_height_reach;
            j["vertical_scale"] = edit_data.vertical_scale;
            j["water_level"] = edit_data.water_level_height;
            j["snow_level"] = edit_data.snow_level_height;
            
            j["tags"] = json::array();
            for(int i=0; i<MAX_TAG_AMOUNT; i++) {
                if(edit_data.tags[i].type == TerrainTagType::DISABLED) continue;
                j["tags"].push_back({
                    {"name", std::string(tag_names[i])},
                    {"uv_x", edit_data.tags[i].uv_x},
                    {"uv_y", edit_data.tags[i].uv_y},
                    {"type", (int)edit_data.tags[i].type}
                });
            }
            std::ofstream o(json_p); o << std::setw(4) << j;
            status_msg = "Saved!"; status_is_error = false; status_timer = 2.0f;
            refresh_level_list();
        } else {
            status_msg = "Save Error!"; status_is_error = true; status_timer = 3.0f;
        }
    }

    void load_level(const std::string& f) {
        std::ifstream i(LEVELS_PATH + f);
        if(!i.good()) return;
        json j; i >> j;
        
        std::string t = j.value("title", "Untitled");
        title_buf =  t.c_str();
        edit_data.resolution_x = j.value("resolution_x", 1024);
        edit_data.resolution_y = j.value("resolution_y", 1024);
        edit_data.minimum_height_reach = j.value("min_height", 0.f);
        edit_data.maximum_height_reach = j.value("max_height", 1000.f);
        edit_data.vertical_scale = j.value("vertical_scale", 1.f);
        edit_data.water_level_height = j.value("water_level", 0.f);
        edit_data.snow_level_height = j.value("snow_level", 3000.f);
        
        std::string path = j.value("heightmap_path", "");
        load_heightmap_to_memory(path);
        
        for(int k=0; k<MAX_TAG_AMOUNT; k++) { edit_data.tags[k].type = TerrainTagType::DISABLED; tag_names[k][0]='\0'; }
        if(j.contains("tags")) {
            int idx = 0;
            for(auto& el : j["tags"]) {
                if(idx>=MAX_TAG_AMOUNT) break;
                std::string n = el["name"];
                tag_names[idx] = n;
                edit_data.tags[idx].uv_x = el["uv_x"];
                edit_data.tags[idx].uv_y = el["uv_y"];
                edit_data.tags[idx].type = (TerrainTagType)el["type"];
                idx++;
            }
        }
        status_msg = "Loaded!"; status_is_error = false; status_timer = 2.0f;
    }
};

#endif