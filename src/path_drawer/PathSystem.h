#ifndef PATHSYSTEM_H
#define PATHSYSTEM_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <queue>
#include <unordered_map>
#include <limits>
#include <algorithm>
#include "Terrain.h"
#include "InputHandler.h"
#include "TerrainLine.h"
#include "Interactable.h"
#include "PathSpacialGrid.h"
#include <set>

using namespace glm;
using namespace std;

enum NodeDestinationType { 
    NO_DEST=0, OPTIONAL=1, NECESSARY=2
};
struct TerrainNode {
    int handle_id;
    vec2 position;
    int connected_system = 0;
    NodeDestinationType destination_type = NodeDestinationType::NO_DEST;
    bool intersection = false;
    bool optional = false;
};
struct TerrainLink {
    int start_handle_id;
    int end_handle_id;
    Line2D *line_obj;
    int link_id = -1;
};

class TerrainPathSystem 
{
protected:
    Terrain *terrain;
    InteractableManager *interactable_manager;
    PathSpatialGrid spatial_grid;

    int node_id = 0, connected_system_number = 0;
    int link_number = 0;
    
    /* graph data */
    std::unordered_map<int, std::vector<int>> adjacency_list;
    vector<TerrainNode> nodes;
    vector<TerrainLink> links;

public:
    TerrainPathSystem(Terrain *t, InteractableManager *im) : terrain(t), interactable_manager(im) {}

    
    int create_path_handle_at_pos (vec2 local_pos, const char* name = "Path Handle") {
        Interactable *i = interactable_manager->create(
            vec3(0.f), name, InteractionType::PATH_HANDLE, INTERACTABLE_INTERACT_DISTANCE, node_id
        );
        float uvx = (local_pos.x+0.5f); // local -0.5 to 0.5, uv is 0-1
        float uvy = (local_pos.y+0.5f); // local -0.5 to 0.5, uv is 0-1
        terrain->attach_to_surface(i, uvx, uvy);
        int id = create_new_node(local_pos);
        cout << "Path handle created, attached to surface at: " << uvx << ", " << uvy << ", id: " << id << endl;
        return id; // will increase in create_new_node fn
    } 

    void add_link(TerrainLink new_link) {
        new_link.link_id = link_number++;
        links.push_back(new_link);

        // Merging Logic: Propagate the smaller ID to the larger ID group
        // ???

        spatial_grid.register_path_segment(new_link.link_id, new_link.line_obj->get_points());
    }

    void remove_link(int id) {
        for (auto l:links) {
            if (l.link_id == id) {
                l.line_obj->set_visible(false);
                spatial_grid.unregister_path_segment(id);
                return;
            }
        }
    }


    int create_new_node(vec2 local_pos) {
        TerrainNode new_node = { node_id++, local_pos, connected_system_number++ };
        nodes.push_back(new_node);
        return nodes.back().handle_id;
    }

    int create_new_destination(const char* name, vec2 local_pos, NodeDestinationType destination_type) {
        int id = create_path_handle_at_pos(local_pos, name);
        nodes.back().destination_type = destination_type;
        return id;
    }

    // bool are_necessary_destinations_connected() {
    //     int current_path_system_id = -1;
    //     for (auto x : nodes) {
    //         if (x.destination_type == NodeDestinationType::NO_DEST) continue;
    //         if (!x.optional) {
    //             if (current_path_system_id == -1) current_path_system_id = x.connected_system;
    //             else if (x.connected_system != current_path_system_id) return false;
    //         }
    //     }
    //     return true;
    // }

    int get_link_at_pos(vec2 pos, float radius, vec2& out_closest_point) {
        return spatial_grid.get_link_at_pos(pos, radius, out_closest_point);
    }

};

#endif

// ===================================================================================================================================

// #ifndef PATHSYSTEM_H
// #define PATHSYSTEM_H

// #include <glad/glad.h>
// #include <GLFW/glfw3.h>
// #include <glm/glm.hpp>
// #include <vector>
// #include <queue>
// #include <unordered_map>
// #include <limits>
// #include <algorithm>
// #include "Terrain.h"
// #include "InputHandler.h"
// #include "TerrainLine.h"
// #include "Interactable.h"

// using namespace glm;
// using namespace std;

// enum NodeDestinationType { 
//     NONE=0, OPTIONAL=1, NECESSARY=2
// }
// struct TerrainNode {
//     int handle_id;
//     vec2 position;
//     int connected_system = 0;
//     NodeDestinationType destination_type = NodeDestinationType::NONE;
//     bool intersection = false;
// };
// struct TerrainLinkData {
//     int start_handle_id;
//     int end_handle_id;

//     std::vector<vec2> points;
// };
// struct TerrainLink {
//     int start_handle_id;
//     int end_handle_id;

//     TerrainLine *line_obj;
// };

// class TerrainPathSystem 
// {
// protected:
//     Terrain *terrain;
//     InteractableManager *interactable_manager;

//     int node_id = 0, connected_system_number = 0;

//     /* graph data */
//     vector<TerrainNode> nodes;
//     vector<TerrainLink> links;

// public:
//     TerrainPathSystem(Terrain *t, InteractableManager *im) : terrain(t), interactable_manager(im) {}

    
//     int create_path_handle_at_pos (vec2 local_pos, const char* name = "Path Handle") {
//         Interactable *i = interactable_manager->create(
//             vec3(0.f), name, InteractionType::PATH_HANDLE, INTERACTABLE_INTERACT_DISTANCE, node_id
//         );
//         float uvx = (local_pos.x+0.5f); // local -0.5 to 0.5, uv is 0-1
//         float uvy = (local_pos.y+0.5f); // local -0.5 to 0.5, uv is 0-1
//         terrain->attach_to_surface(i, uvx, uvy);
//         int id = create_new_node(local_pos);
//         cout << "Path handle created, attached to surface at: " << uvx << ", " << uvy << ", id: " << id << endl;
//         return id; // will increase in create_new_node fn
//     } 

//     void add_link(TerrainLinkData new_link_data) {
//         TerrainLine *line = new TerrainLine(terrain->terrain_data, new_link_data.points);
//         line->set_parent(terrain->terrain_obj);
//         TerrainLink new_link = { new_link_data.start_handle_id, new_link_data.end_handle_id, line };
//         links.push_back(new_link);

//         // Merging Logic: Propagate the smaller ID to the larger ID group
//         // ???
//     }

//     int create_new_node(vec2 local_pos) {
//         TerrainNode new_node = { node_id++, local_pos, connected_system_number++ };
//         nodes.push_back(new_node);
//         return nodes.back().handle_id;
//     }

//     int create_new_destination(const char* name, vec2 local_pos, NodeDestinationType destination_type) {
//         int id = create_path_handle_at_pos(local_pos, name);
//         nodes.back().destination_type = destination_type;
//     }

//     bool are_necessary_destinations_connected() {
//         int current_path_system_id = -1;
//         for (auto x : nodes) {
//             if (x.destination_type == NodeDestinationType::NONE) continue;
//             if (!x->optional) {
//                 if (current_path_system_id == -1) current_path_system_id = x.connected_system;
//                 else if (x.connected_system != current_path_system_id) return false;
//             }
//         }
//         return true;
//     }
// };

// #endif

// ===================================================================================================================================
// #ifndef PATHSYSTEM_H
// #define PATHSYSTEM_H

// #include <glad/glad.h>
// #include <GLFW/glfw3.h>
// #include <glm/glm.hpp>
// #include <vector>
// #include <queue>
// #include <unordered_map>
// #include <limits>
// #include <algorithm>
// #include "Terrain.h"
// #include "InputHandler.h"
// #include "Line.h"

// using namespace glm;
// using namespace std;

// struct Destination {
//     const char* name;
//     int id, path_system_id;
//     bool optional;
//     Destination *connected_one = nullptr;
//     Destination *connected_two = nullptr;
// };

// // Represents an Edge in the graph
// struct Node {
//     int destination_id_a, destination_id_b;
//     float link_length;
//     bool intersection = false;
// };
// struct TerrainLink {
//     int link_id = -1;
//     int start_handle_id = -1;
//     int end_handle_id = -1;
//     std::unique_ptr<TerrainLine*> line_object = nullptr;
// };

// class PathSystem
// {
// private:
//     vector<Destination*> destinations;
//     vector<Node> nodes;
//     vector<TerrainLink> path_links;
//     int current_path_system_num = 0;
//     int next_path_link_id = 1;
//     Terrain *terrain;

//     // Helper to build adjacency list for Dijkstra: map<node_id, vector<pair<neighbor_id, cost>>>
//     unordered_map<int, vector<pair<int, float>>> get_adjacency_list() {
//         unordered_map<int, vector<pair<int, float>>> adj;
//         for (const auto& edge : nodes) {
//             adj[edge.destination_id_a].push_back({ edge.destination_id_b, edge.link_length });
//             adj[edge.destination_id_b].push_back({ edge.destination_id_a, edge.link_length });
//         }
//         return adj;
//     }

// public:
//     PathSystem(Terrain *t) : terrain(t) {}

//     TerrainLink* create_path_link(int from_handle_id, int to_handle_id, TerrainLine* line_obj) {
//         // Calculate length from points (simple sum of segments for now)
//         float length = 0.0f;
//         for (size_t i = 0; i < points.size() - 1; ++i) {
//             length += glm::length(points[i+1] - points[i]);
//         }

//         TerrainLink link = { 
//             next_path_link_id++, 
//             from_handle_id, to_handle_id, 
//             std::unique_ptr<TerrainLine>(line_obj), 
//         };
//         path_links.push_back(std::move(link));
        
//         // Update PathSystem graph with the new edge
//         add_link(from_handle_id, to_handle_id, length, path_links.back().id);
        
//         return &path_links.back();
//     }

//     void delete_path_link(TerrainLink* link) {
//         if (!link) return;
//         int path_id = link->id;
        
//         // 1. Remove from path_links (geometry)
//         auto it_link = std::remove_if(path_links.begin(), path_links.end(), 
//             [path_id](const TerrainLink& l) { return l.id == path_id; });
//         if (it_link != path_links.end()) path_links.erase(it_link, path_links.end());
        
//         // 2. Remove from path_system (graph edges)
//         auto it_node = std::remove_if(nodes.begin(), nodes.end(), 
//             [path_id](const Node& n) { return n.terrain_path_id == path_id; });
//         if (it_node != nodes.end()) nodes.erase(it_node, nodes.end());
//     }

//     bool is_traversable(int destination_id_a, int destination_id_b) {
//         Destination* a = get_destination(destination_id_a);
//         Destination* b = get_destination(destination_id_b);
//         return a && b && a->path_system_id == b->path_system_id;
//     }

//     float find_traverse_length(int destination_id_a, int destination_id_b) {
//         // Run Dijkstra but only return distance
//         if (!is_traversable(destination_id_a, destination_id_b)) return -1.0f;
        
//         // Priority queue stores {cost, node_id}, ordered by lowest cost
//         priority_queue<pair<float, int>, vector<pair<float, int>>, greater<pair<float, int>>> pq;
//         unordered_map<int, float> dist;
//         auto adj = get_adjacency_list();

//         // Initialize distances
//         for (auto d : destinations) dist[d->id] = std::numeric_limits<float>::infinity();

//         dist[destination_id_a] = 0.0f;
//         pq.push({ 0.0f, destination_id_a });

//         while (!pq.empty()) {
//             float d = pq.top().first;
//             int u = pq.top().second;
//             pq.pop();

//             if (u == destination_id_b) return d; // Target found
//             if (d > dist[u]) continue;

//             for (auto& edge : adj[u]) {
//                 int v = edge.first;
//                 float weight = edge.second;
//                 if (dist[u] + weight < dist[v]) {
//                     dist[v] = dist[u] + weight;
//                     pq.push({ dist[v], v });
//                 }
//             }
//         }
//         return -1.0f; // Path not found (shouldn't happen if is_traversable is true)
//     }

//     vector<int> find_traverse_nodes(int destination_id_a, int destination_id_b) {
//         if (!is_traversable(destination_id_a, destination_id_b)) return {};

//         priority_queue<pair<float, int>, vector<pair<float, int>>, greater<pair<float, int>>> pq;
//         unordered_map<int, float> dist;
//         unordered_map<int, int> prev; // To reconstruct path
//         auto adj = get_adjacency_list();

//         for (auto d : destinations) dist[d->id] = std::numeric_limits<float>::infinity();

//         dist[destination_id_a] = 0.0f;
//         pq.push({ 0.0f, destination_id_a });
//         prev[destination_id_a] = -1;

//         bool found = false;

//         while (!pq.empty()) {
//             float d = pq.top().first;
//             int u = pq.top().second;
//             pq.pop();

//             if (u == destination_id_b) {
//                 found = true;
//                 break;
//             }

//             if (d > dist[u]) continue;

//             for (auto& edge : adj[u]) {
//                 int v = edge.first;
//                 float weight = edge.second;
//                 if (dist[u] + weight < dist[v]) {
//                     dist[v] = dist[u] + weight;
//                     prev[v] = u;
//                     pq.push({ dist[v], v });
//                 }
//             }
//         }

//         vector<int> path;
//         if (found) {
//             for (int at = destination_id_b; at != -1; at = prev[at]) {
//                 path.push_back(at);
//             }
//             std::reverse(path.begin(), path.end());
//         }
//         return path;
//     }

//     bool are_necessary_destinations_connected() {
//         int current_path_system_id = -1;
//         for (auto x : destinations) {
//             if (!x->optional) {
//                 if (current_path_system_id == -1) current_path_system_id = x->path_system_id;
//                 else if (x->path_system_id != current_path_system_id) return false;
//             }
//         }
//         return true;
//     }

//     bool are_all_destinations_connected() {
//         int current_path_system_id = -1;
//         for (auto x : destinations) {
//             if (current_path_system_id == -1) current_path_system_id = x->path_system_id;
//             else if (x->path_system_id != current_path_system_id) return false;
//         }
//         return true;
//     }

//     void add_link(int id_1, int id_2, float length) {
//         Destination* a = get_destination(id_1);
//         Destination* b = get_destination(id_2);

//         if (a && b) {
//             Node n = { a->id, b->id, length };
//             nodes.push_back(n);

//             // Merging Logic: Propagate the smaller ID to the larger ID group
//             if (a->path_system_id != b->path_system_id) {
//                 int old_id = glm::max(a->path_system_id, b->path_system_id);
//                 int new_id = glm::min(a->path_system_id, b->path_system_id);
                
//                 for (auto* d : destinations) {
//                     if (d->path_system_id == old_id) {
//                         d->path_system_id = new_id;
//                     }
//                 }
//             }
//         }
//     }

//     void create_new_node(vec2 local_pos) {
//         node
//     }

//     void create_destination(Interactable* i, bool optional = false) {
//         Destination* d = new Destination{ i->name, i->get_id(), current_path_system_num++, optional };
//         destinations.push_back(d);
//     }

//     Interactable* create_path_handle_at_pos (vec3 local_pos) {
//         Interactable *i = new Interactable(
//             vec3(0.f), "Path Handle", InteractionType::PATH_HANDLE, INTERACTABLE_INTERACT_DISTANCE
//         );
//         float uvx = (local_pos.x+0.5f); // local -0.5 to 0.5, uv is 0-1
//         float uvy = (local_pos.y+0.5f); // local -0.5 to 0.5, uv is 0-1
//         terrain->attach_to_surface(i, uvx, uvy);
//         int id = create_new_node(vec2(local_pos));
//         cout << "Path handle created, attached to surface at: " << uvx << ", " << uvy << ", id: " << id << endl;
//         return i;
//     } 

//     Destination* get_destination(int id) {
//         for (auto d : destinations) {
//             if (d->id == id) return d;
//         }
//         return nullptr;
//     }
// };

// #endif