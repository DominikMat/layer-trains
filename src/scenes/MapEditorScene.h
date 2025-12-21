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
#include "Scene.h"
#include "TerrainData.h"
#include "Texture.h" 
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "json.hpp"

// Image writing for heightmap saving
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#endif

// Image loading for initial heightmap load
//#include "stb_image.h" 

namespace fs = std::filesystem;

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

    // Painter State
    std::vector<unsigned char> heightmap_pixels; // Raw 8-bit grayscale data
    GLuint heightmap_tex_id = 0;
    int hmap_width = 1024;
    int hmap_height = 1024;
    
    float brush_size = 50.0f;
    float brush_strength = 2.0f; // Height change per frame

    // GUI State
    char title_buf[128] = "";
    char tag_names[MAX_TAG_AMOUNT][64];
    
    // Import/Save State
    std::vector<std::string> available_levels;
    int selected_level_idx = -1;
    
    // Feedback
    std::string status_msg = "";
    float status_timer = 0.0f;
    bool status_is_error = false;

public:

    MapEditorScene (GLFWwindow *win, World *w, Camera *c, ScreenUI *s, InputHandler *ih) 
        : Scene(w,c,s,ih), window(win) 
    {
        // 1. Ensure directories exist
        if (!fs::exists(LEVELS_PATH)) fs::create_directories(LEVELS_PATH);
        if (!fs::exists(HEIGHTMAPS_PATH)) fs::create_directories(HEIGHTMAPS_PATH);

        // 2. Load Default Data
        edit_data = terrain_transalpine; 
        
        // 3. Load Initial Heightmap into Memory for Painting
        load_heightmap_to_memory(edit_data.heightmap_path);
        
        sync_buffers_from_struct();
        refresh_level_list();
    }
    
    void init( ) override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    void loop(float dt) override {
        // Prevent crash on minimize (width/height = 0)
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        if (w == 0 || h == 0) return;

        // Update Timers
        if (status_timer > 0.0f) status_timer -= dt;

        // Render World (Optional, if you want to see the 3D preview behind UI)
        // world->render(); 

        // ImGui Frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        draw_left_panel();  // Painter
        draw_right_panel(); // Settings

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    ~MapEditorScene() {
        if (heightmap_tex_id != 0) glDeleteTextures(1, &heightmap_tex_id);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

private:

    // =========================================================
    // PANELS
    // =========================================================

    void draw_left_panel() {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        float panel_width = 450.0f;
        
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y));
        ImGui::SetNextWindowSize(ImVec2(panel_width, viewport->Size.y));
        
        ImGui::Begin("Heightmap Painter", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        ImGui::Text("Brush Settings");
        ImGui::SliderFloat("Size", &brush_size, 1.0f, 200.0f);
        ImGui::SliderFloat("Strength", &brush_strength, 0.1f, 10.0f);
        
        ImGui::Separator();
        ImGui::Text("2D View (Left: Raise, Right: Lower)");

        // Display Texture
        ImVec2 avail = ImGui::GetContentRegionAvail();
        float aspect = (float)hmap_width / (float)hmap_height;
        float image_w = avail.x;
        float image_h = image_w / aspect;

        ImVec2 p_min = ImGui::GetCursorScreenPos();
        ImGui::Image((void*)(intptr_t)heightmap_tex_id, ImVec2(image_w, image_h));
        
        // Handle Input
        if (ImGui::IsItemHovered()) {
            // Calculate UV of mouse
            ImVec2 mouse_pos = ImGui::GetMousePos();
            float rel_x = mouse_pos.x - p_min.x;
            float rel_y = mouse_pos.y - p_min.y;
            
            float u = rel_x / image_w;
            float v = rel_y / image_h;

            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                apply_brush(u, v, true);
            }
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                apply_brush(u, v, false);
            }
        }

        ImGui::End();
    }

    void draw_right_panel() {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        float panel_width = 400.0f;
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x - panel_width, viewport->Pos.y));
        ImGui::SetNextWindowSize(ImVec2(panel_width, viewport->Size.y));
        
        ImGui::Begin("Terrain Properties", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        // --- Header / Save / Load ---
        if (ImGui::Button("Save Level", ImVec2(120, 30))) save_level();
        
        ImGui::SameLine();
        
        // Import Dropdown
        ImGui::PushItemWidth(150);
        const char* preview = (selected_level_idx >= 0 && selected_level_idx < available_levels.size()) 
                              ? available_levels[selected_level_idx].c_str() 
                              : "Select Level...";
        if (ImGui::BeginCombo("##import", preview)) {
            for (int i = 0; i < available_levels.size(); i++) {
                const bool is_selected = (selected_level_idx == i);
                if (ImGui::Selectable(available_levels[i].c_str(), is_selected)) {
                    selected_level_idx = i;
                    load_level(available_levels[i]);
                }
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
        
        ImGui::SameLine();
        if (ImGui::Button("R")) refresh_level_list(); // Refresh button

        // Feedback Message
        if (status_timer > 0.0f) {
            ImVec4 col = status_is_error ? ImVec4(1, 0, 0, 1) : ImVec4(0, 1, 0, 1);
            ImGui::TextColored(col, "%s", status_msg.c_str());
        } else {
            ImGui::Text(""); // Spacer
        }

        ImGui::Separator();

        // --- Metadata ---
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Level Title");
        ImGui::InputText("##Title", title_buf, sizeof(title_buf));
        
        // Auto-generated paths display (Read-only)
        std::string safe_title = std::string(title_buf);
        std::replace(safe_title.begin(), safe_title.end(), ' ', '_');
        ImGui::TextDisabled("Save Path: %s%s.json", LEVELS_PATH.c_str(), safe_title.c_str());

        ImGui::Separator();

        // --- Physics ---
        if (ImGui::CollapsingHeader("Dimensions & Scale", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Validation: Keep > 0
            if (ImGui::InputInt("Res X", &edit_data.resolution_x)) 
                edit_data.resolution_x = std::max(128, edit_data.resolution_x);
            if (ImGui::InputInt("Res Y", &edit_data.resolution_y)) 
                edit_data.resolution_y = std::max(128, edit_data.resolution_y);
            
            ImGui::DragFloat("Min Height", &edit_data.minimum_height_reach, 1.0f, 0.0f, 10000.0f);
            ImGui::DragFloat("Max Height", &edit_data.maximum_height_reach, 1.0f, 0.0f, 10000.0f);
            
            // Scale shouldn't be negative
            if (ImGui::InputFloat("Vertical Scale", &edit_data.vertical_scale, 0.0001f, 0.001f, "%.6f"))
                edit_data.vertical_scale = std::max(0.00001f, edit_data.vertical_scale);
        }

        // --- Environment ---
        if (ImGui::CollapsingHeader("Environment", ImGuiTreeNodeFlags_DefaultOpen)) {
             ImGui::SliderFloat("Water Level", &edit_data.water_level_height, edit_data.minimum_height_reach, edit_data.maximum_height_reach);
             ImGui::SliderFloat("Snow Level", &edit_data.snow_level_height, edit_data.minimum_height_reach, edit_data.maximum_height_reach);
        }

        // --- Tags ---
        if (ImGui::CollapsingHeader("Terrain Tags", ImGuiTreeNodeFlags_DefaultOpen)) {
            
            // List active tags
            for (int i = 0; i < MAX_TAG_AMOUNT; i++) {
                if (edit_data.tags[i].type == TerrainTagType::DISABLED) continue;

                ImGui::PushID(i);
                
                // Delete Button
                if (ImGui::Button("X")) {
                    edit_data.tags[i].type = TerrainTagType::DISABLED;
                    tag_names[i][0] = '\0';
                }
                ImGui::SameLine();

                // Tree Node for details
                if (ImGui::TreeNode(tag_names[i][0] == '\0' ? "Unnamed Tag" : tag_names[i])) {
                    
                    // Type
                    const char* types[] = { "Name Tag", "Level Start", "Level End", "Disabled" };
                    int current_type = (int)edit_data.tags[i].type;
                    if (ImGui::Combo("Type", &current_type, types, IM_ARRAYSIZE(types))) {
                        edit_data.tags[i].type = (TerrainTagType)current_type;
                    }

                    // Name
                    ImGui::InputText("Label", tag_names[i], sizeof(tag_names[i]));

                    // Position
                    ImGui::SliderFloat("UV X", &edit_data.tags[i].uv_x, 0.0f, 1.0f);
                    ImGui::SliderFloat("UV Y", &edit_data.tags[i].uv_y, 0.0f, 1.0f);
                    
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }

            // Add New Tag Button
            if (ImGui::Button("+ Add New Tag", ImVec2(-1, 0))) {
                for (int i = 0; i < MAX_TAG_AMOUNT; i++) {
                    if (edit_data.tags[i].type == TerrainTagType::DISABLED) {
                        edit_data.tags[i].type = TerrainTagType::NAME_TAG;
                        edit_data.tags[i].uv_x = 0.5f;
                        edit_data.tags[i].uv_y = 0.5f;
                        strncpy(tag_names[i], "New Tag", 64);
                        break;
                    }
                }
            }
        }

        ImGui::End();
    }

    // =========================================================
    // LOGIC
    // =========================================================

    void apply_brush(float u, float v, bool raise) {
        if (heightmap_pixels.empty()) return;

        int center_x = (int)(u * hmap_width);
        int center_y = (int)(v * hmap_height);
        int radius = (int)brush_size;
        int radius_sq = radius * radius;
        
        bool changed = false;

        // Bounding box iteration
        for (int y = center_y - radius; y <= center_y + radius; y++) {
            for (int x = center_x - radius; x <= center_x + radius; x++) {
                
                // Check Bounds
                if (x < 0 || x >= hmap_width || y < 0 || y >= hmap_height) continue;

                // Circular Brush
                int dx = x - center_x;
                int dy = y - center_y;
                if (dx*dx + dy*dy > radius_sq) continue;

                // Falloff (Soft Brush)
                float dist = sqrt(dx*dx + dy*dy);
                float falloff = 1.0f - (dist / radius);
                falloff = falloff * falloff; // Quad curve

                // Apply Height
                int idx = (y * hmap_width + x); // 1 channel
                float val = (float)heightmap_pixels[idx];
                float change = brush_strength * falloff;
                
                if (raise) val += change;
                else val -= change;

                val = std::clamp(val, 0.0f, 255.0f);
                
                if (heightmap_pixels[idx] != (unsigned char)val) {
                    heightmap_pixels[idx] = (unsigned char)val;
                    changed = true;
                }
            }
        }

        if (changed) update_texture();
    }

    void load_heightmap_to_memory(const std::string& path) {
        int w, h, ch;
        // Use stbi directly to get raw bytes
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 1); // Force 1 channel (grayscale)
        
        if (data) {
            hmap_width = w;
            hmap_height = h;
            heightmap_pixels.assign(data, data + (w * h));
            stbi_image_free(data);
        } else {
            // Create fallback blank texture if file missing
            hmap_width = 1024;
            hmap_height = 1024;
            heightmap_pixels.assign(hmap_width * hmap_height, 0); // Black
            std::cout << "Warning: Could not load heightmap, creating blank." << std::endl;
        }

        // Generate OpenGL Texture
        if (heightmap_tex_id == 0) glGenTextures(1, &heightmap_tex_id);
        update_texture();
    }

    void update_texture() {
        glBindTexture(GL_TEXTURE_2D, heightmap_tex_id);
        // Using GL_RED for single channel visualization. 
        // Note: For ImGui::Image to render it as grayscale (not red), we might want to use swizzle mask 
        // or just upload as RGB. For simplicity, let's use RGB here by duplicating bytes, 
        // OR better: use GL_RED and rely on a shader. But ImGui uses default shaders. 
        // Easiest "lazy" fix for visualization: upload as GL_RGB.
        
        // Temp RGB buffer for visualization
        std::vector<unsigned char> vis_buffer(hmap_width * hmap_height * 3);
        for(size_t i=0; i < heightmap_pixels.size(); i++) {
            vis_buffer[i*3+0] = heightmap_pixels[i];
            vis_buffer[i*3+1] = heightmap_pixels[i];
            vis_buffer[i*3+2] = heightmap_pixels[i];
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, hmap_width, hmap_height, 0, GL_RGB, GL_UNSIGNED_BYTE, vis_buffer.data());
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    // =========================================================
    // IO OPERATIONS
    // =========================================================

    void refresh_level_list() {
        available_levels.clear();
        if (fs::exists(LEVELS_PATH)) {
            for (const auto& entry : fs::directory_iterator(LEVELS_PATH)) {
                if (entry.path().extension() == ".json") {
                    available_levels.push_back(entry.path().filename().string());
                }
            }
        }
    }

    void sync_buffers_from_struct() {
        if (edit_data.title) strncpy(title_buf, edit_data.title, sizeof(title_buf));
        
        for(int i=0; i<MAX_TAG_AMOUNT; i++) {
             if (edit_data.tags[i].name)
                 strncpy(tag_names[i], edit_data.tags[i].name, sizeof(tag_names[i]));
             else 
                 tag_names[i][0] = '\0';
        }
    }

    void save_level() {
        // 1. Prepare Paths based on Title
        std::string clean_title = title_buf;
        if (clean_title.empty()) clean_title = "untitled_level";
        std::replace(clean_title.begin(), clean_title.end(), ' ', '_');

        std::string json_filename = clean_title + ".json";
        std::string hmap_filename = clean_title + "_heightmap.png";
        
        std::string full_json_path = LEVELS_PATH + json_filename;
        std::string full_hmap_path = HEIGHTMAPS_PATH + hmap_filename;

        // 2. Save PNG
        // We write the raw grayscale data. Stride is width * 1 byte.
        int success = stbi_write_png(full_hmap_path.c_str(), hmap_width, hmap_height, 1, heightmap_pixels.data(), hmap_width);
        
        if (!success) {
            status_msg = "Error Saving Heightmap!";
            status_is_error = true;
            status_timer = 3.0f;
            return;
        }

        // 3. Update Struct Paths (Important: The game will load from these paths)
        // Note: In a real engine, we'd handle strings better than const char* assignment.
        // For now, we assume these paths are re-constructed on load or used transiently.
        // We WON'T update the edit_data pointers here directly to point to local stack strings, 
        // but we will write the correct paths to the JSON.

        // 4. Save JSON
        json j;
        j["title"] = std::string(title_buf);
        j["heightmap_path"] = full_hmap_path;
        j["areas_data_path"] = std::string(edit_data.areas_data_path ? edit_data.areas_data_path : ""); // We don't edit this yet
        j["resolution_x"] = edit_data.resolution_x;
        j["resolution_y"] = edit_data.resolution_y;
        j["min_height"] = edit_data.minimum_height_reach;
        j["max_height"] = edit_data.maximum_height_reach;
        j["vertical_scale"] = edit_data.vertical_scale;
        j["water_level"] = edit_data.water_level_height;
        j["snow_level"] = edit_data.snow_level_height;

        j["tags"] = json::array();
        for(int i=0; i<MAX_TAG_AMOUNT; i++) {
            if (edit_data.tags[i].type == TerrainTagType::DISABLED) continue;
            j["tags"].push_back({
                {"name", std::string(tag_names[i])},
                {"uv_x", edit_data.tags[i].uv_x},
                {"uv_y", edit_data.tags[i].uv_y},
                {"type", (int)edit_data.tags[i].type}
            });
        }

        std::ofstream o(full_json_path);
        o << std::setw(4) << j << std::endl;
        o.close();

        // 5. Feedback
        status_msg = "Level Saved Successfully!";
        status_is_error = false;
        status_timer = 2.0f;
        
        refresh_level_list();
    }

    void load_level(const std::string& filename) {
        std::string full_path = LEVELS_PATH + filename;
        std::ifstream i(full_path);
        if (!i.good()) {
            status_msg = "Failed to load JSON!";
            status_is_error = true;
            status_timer = 3.0f;
            return;
        }

        json j;
        i >> j;

        // JSON -> Buffers
        std::string s_title = j.value("title", "Untitled");
        strncpy(title_buf, s_title.c_str(), sizeof(title_buf));

        // JSON -> Struct
        edit_data.resolution_x = j.value("resolution_x", 1024);
        edit_data.resolution_y = j.value("resolution_y", 1024);
        edit_data.minimum_height_reach = j.value("min_height", 0.f);
        edit_data.maximum_height_reach = j.value("max_height", 1000.f);
        edit_data.vertical_scale = j.value("vertical_scale", 1.f);
        edit_data.water_level_height = j.value("water_level", 0.f);
        edit_data.snow_level_height = j.value("snow_level", 3000.f);
        
        // Note: We need to store the path strings somewhere persistent if we want to use them
        // For this editor logic, loading the heightmap pixels immediately is what matters.
        std::string hmap_path = j.value("heightmap_path", "");
        load_heightmap_to_memory(hmap_path);

        // Tags
        for(int k=0; k<MAX_TAG_AMOUNT; k++) {
            edit_data.tags[k].type = TerrainTagType::DISABLED;
            tag_names[k][0] = '\0';
        }
        if (j.contains("tags")) {
            int idx = 0;
            for (auto& element : j["tags"]) {
                if (idx >= MAX_TAG_AMOUNT) break;
                std::string t_name = element["name"];
                strncpy(tag_names[idx], t_name.c_str(), sizeof(tag_names[idx]));
                edit_data.tags[idx].uv_x = element["uv_x"];
                edit_data.tags[idx].uv_y = element["uv_y"];
                edit_data.tags[idx].type = (TerrainTagType)element["type"];
                idx++;
            }
        }
        
        status_msg = "Level Loaded!";
        status_is_error = false;
        status_timer = 2.0f;
    }
};

#endif