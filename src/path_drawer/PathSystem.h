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
#include "Line.h"

using namespace glm;
using namespace std;

struct Destination {
    const char* name;
    int id, path_system_id;
    bool optional;
    Destination *connected_one = nullptr;
    Destination *connected_two = nullptr;
};

// Represents an Edge in the graph
struct Node {
    int destination_id_a, destination_id_b;
    float link_length;
};

class PathSystem
{
private:
    vector<Destination*> destinations;
    vector<Node> path_system; // Functions as an edge list
    int current_path_system_num = 0;

    // Helper to build adjacency list for Dijkstra: map<node_id, vector<pair<neighbor_id, cost>>>
    unordered_map<int, vector<pair<int, float>>> get_adjacency_list() {
        unordered_map<int, vector<pair<int, float>>> adj;
        for (const auto& edge : path_system) {
            adj[edge.destination_id_a].push_back({ edge.destination_id_b, edge.link_length });
            adj[edge.destination_id_b].push_back({ edge.destination_id_a, edge.link_length });
        }
        return adj;
    }

public:
    PathSystem() {}

    bool is_traversable(int destination_id_a, int destination_id_b) {
        Destination* a = get_destination(destination_id_a);
        Destination* b = get_destination(destination_id_b);
        return a && b && a->path_system_id == b->path_system_id;
    }

    float find_traverse_length(int destination_id_a, int destination_id_b) {
        // Run Dijkstra but only return distance
        if (!is_traversable(destination_id_a, destination_id_b)) return -1.0f;
        
        // Priority queue stores {cost, node_id}, ordered by lowest cost
        priority_queue<pair<float, int>, vector<pair<float, int>>, greater<pair<float, int>>> pq;
        unordered_map<int, float> dist;
        auto adj = get_adjacency_list();

        // Initialize distances
        for (auto d : destinations) dist[d->id] = std::numeric_limits<float>::infinity();

        dist[destination_id_a] = 0.0f;
        pq.push({ 0.0f, destination_id_a });

        while (!pq.empty()) {
            float d = pq.top().first;
            int u = pq.top().second;
            pq.pop();

            if (u == destination_id_b) return d; // Target found
            if (d > dist[u]) continue;

            for (auto& edge : adj[u]) {
                int v = edge.first;
                float weight = edge.second;
                if (dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    pq.push({ dist[v], v });
                }
            }
        }
        return -1.0f; // Path not found (shouldn't happen if is_traversable is true)
    }

    vector<int> find_traverse_nodes(int destination_id_a, int destination_id_b) {
        if (!is_traversable(destination_id_a, destination_id_b)) return {};

        priority_queue<pair<float, int>, vector<pair<float, int>>, greater<pair<float, int>>> pq;
        unordered_map<int, float> dist;
        unordered_map<int, int> prev; // To reconstruct path
        auto adj = get_adjacency_list();

        for (auto d : destinations) dist[d->id] = std::numeric_limits<float>::infinity();

        dist[destination_id_a] = 0.0f;
        pq.push({ 0.0f, destination_id_a });
        prev[destination_id_a] = -1;

        bool found = false;

        while (!pq.empty()) {
            float d = pq.top().first;
            int u = pq.top().second;
            pq.pop();

            if (u == destination_id_b) {
                found = true;
                break;
            }

            if (d > dist[u]) continue;

            for (auto& edge : adj[u]) {
                int v = edge.first;
                float weight = edge.second;
                if (dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    prev[v] = u;
                    pq.push({ dist[v], v });
                }
            }
        }

        vector<int> path;
        if (found) {
            for (int at = destination_id_b; at != -1; at = prev[at]) {
                path.push_back(at);
            }
            std::reverse(path.begin(), path.end());
        }
        return path;
    }

    bool are_necessary_destinations_connected() {
        int current_path_system_id = -1;
        for (auto x : destinations) {
            if (!x->optional) {
                if (current_path_system_id == -1) current_path_system_id = x->path_system_id;
                else if (x->path_system_id != current_path_system_id) return false;
            }
        }
        return true;
    }

    bool are_all_destinations_connected() {
        int current_path_system_id = -1;
        for (auto x : destinations) {
            if (current_path_system_id == -1) current_path_system_id = x->path_system_id;
            else if (x->path_system_id != current_path_system_id) return false;
        }
        return true;
    }

    void add_link(int id_1, int id_2, float length) {
        Destination* a = get_destination(id_1);
        Destination* b = get_destination(id_2);

        if (a && b) {
            Node n = { a->id, b->id, length };
            path_system.push_back(n);
            
            // Legacy pointer updates (Note: overwrites previous connections)
            a->connected_two = b;
            b->connected_one = a;

            // Merging Logic: Propagate the smaller ID to the larger ID group
            if (a->path_system_id != b->path_system_id) {
                int old_id = glm::max(a->path_system_id, b->path_system_id);
                int new_id = glm::min(a->path_system_id, b->path_system_id);
                
                for (auto* d : destinations) {
                    if (d->path_system_id == old_id) {
                        d->path_system_id = new_id;
                    }
                }
            }
        }
    }

    void create_destination(Interactable* i, bool optional = false) {
        Destination* d = new Destination{ i->name, i->get_id(), current_path_system_num++, optional };
        destinations.push_back(d);
    }

    Destination* get_destination(int id) {
        for (auto d : destinations) {
            if (d->id == id) return d;
        }
        return nullptr;
    }
};

#endif