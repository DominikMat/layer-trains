#pragma once

#include <glm/glm.hpp>
#include <vector>

#define V3_ONE glm::vec3(1.0f,1.0f,1.0f)
#define V3_X glm::vec3(1.0f,0.0f,0.0f)
#define V3_Y glm::vec3(0.0f,1.0f,0.0f)
#define V3_Z glm::vec3(0.0f,0.0f,1.0f)

void print_vector_v2 (std::vector<glm::vec2> v, const char* name) {
    std::cout << name << ": " << std::endl;
    for (size_t i=0; i<v.size() && i < 3; i++) // Loguj tylko kilka pierwszych
    std::cout << "      -> position" << i << ": (" << v[i].x << ", " << v[i].y << ")" << std::endl; 
    if (v.size() > 3)
    std::cout << "      ... " << std::endl << std::endl;
}

void print_vector_v3 (std::vector<glm::vec3> v, const char* name) {
    std::cout << name << ": " << std::endl;
    for (size_t i=0; i<v.size() && i < 3; i++) // Loguj tylko kilka pierwszych
    std::cout << "      -> position" << i << ": (" << v[i].x << ", " << v[i].y << ", " << v[i].z  << ")" << std::endl; 
    if (v.size() > 3)
    std::cout << "      ... " << std::endl << std::endl;
}

void print_v3 (glm::vec3 v, const char* name = "Vector") {
    std::cout << name << ": (" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
}

// #define print (a) std::cout << a << std::endl;
// #define print2 (a,b) std::cout << a << b << std::endl;
// #define print3 (a,b,c) std::cout << a << b << c << std::endl;
// #define print4 (a,b,c,d) std::cout << a << b << c << d << std::endl;