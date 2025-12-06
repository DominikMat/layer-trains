cmake_minimum_required(VERSION 3.16)
project(Layer_Trains)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FREETYPE_DIR "C:/Media/Projects/OpenGL/Libraries/freetype-2.14.1") 
set(FREETYPE_INCLUDE_DIR_freetype2 "${FREETYPE_DIR}/include")
set(FREETYPE_INCLUDE_DIR_ft2build "${FREETYPE_DIR}/include")
# set(FREETYPE_LIBRARY_RELEASE "${FREETYPE_DIR}/release/freetype.lib")
# set(FREETYPE_LIBRARY_DEBUG "${FREETYPE_DIR}/debug/freetype.lib")

# (opcjonalne) ułatwienie: wybierz Release domyślnie tylko w generatorach multi-config
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Znajdź paczki z vcpkg (vcpkg musi być podany jako toolchain file w wywołaniu cmake)
find_package(glfw3 CONFIG REQUIRED)    # target: glfw
find_package(glad CONFIG REQUIRED)     # target: glad::glad
find_package(OpenGL REQUIRED)          # target: OpenGL::GL
find_package(Freetype REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

include_directories(${FREETYPE_INCLUDE_DIRS})

if(FREETYPE_FOUND)
    # Use the modern target if available, otherwise use variables
    if(TARGET Freetype::Freetype)
        target_link_libraries(Layer_Trains PRIVATE Freetype::Freetype)
    else()
        target_link_libraries(Layer_Trains PRIVATE ${FREETYPE_LIBRARIES})
    endif()
endif()

# linkujemy prywatnie: nie musisz dodawać include dirs ręcznie jeśli paczki z vcpkg dostarczają konfigurację
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        glfw
        glad::glad
        OpenGL::GL
)

target_include_directories(Layer_Trains
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src           # jeśli nagłówki leżą w src/...
        ${CMAKE_SOURCE_DIR}/src/world_objects
        ${CMAKE_SOURCE_DIR}/src/rendering
        ${CMAKE_SOURCE_DIR}/src/textures
        ${CMAKE_SOURCE_DIR}/src/shaders
        ${CMAKE_SOURCE_DIR}/src/user_interaction
        ${CMAKE_SOURCE_DIR}/src/settings
        ${CMAKE_SOURCE_DIR}/src/ui
        ${CMAKE_SOURCE_DIR}/src/terrain
        ${CMAKE_SOURCE_DIR}/src/path_drawer
        ${CMAKE_SOURCE_DIR}/src/scenes
)

# (opcjonalnie) jeśli chcesz wydruki configure-time
message(STATUS "CMake generator: ${CMAKE_GENERATOR}")
message(STATUS "CMake build type: ${CMAKE_BUILD_TYPE}")