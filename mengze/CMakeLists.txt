cmake_minimum_required(VERSION 3.16)

# create sample app project
project(mengze LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories("${PROJECT_SOURCE_DIR}")
add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)
add_executable(${PROJECT_NAME} "main.cpp" "core/application.cpp" "core/application.h" "core/logging.h" "core/imgui_build.cpp" "core/layer.h" "core/image.cpp" "core/image.h" "rendering/renderer.cpp" "rendering/renderer.h" "rendering/camera.h" "rendering/camera.cpp" "core/input/input.h" "core/input/input.cpp" "core/input/key_codes.h" "rendering/render_layer.cpp" "rendering/render_layer.h" "scene_graph/scene.h" "scene_graph/scene.cpp" "scene_graph/node.cpp" "scene_graph/components/transform.h" "scene_graph/components/transform.cpp" "scene_graph/component.h"    "zbuffer.h" "visible_surface_determination/scanline_zbuffer.h" "visible_surface_determination/edge_table.h" "visible_surface_determination/geometry.h" "visible_surface_determination/geometry.cpp")

# Link third party libraries
target_link_libraries(${PROJECT_NAME} PUBLIC
    volk
    glm
    stb
    imgui
    tinygltf
    spdlog
    ctpl
    glfw)
