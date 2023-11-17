cmake_minimum_required(VERSION 3.16)

# create sample app project
project(mengze LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories("${PROJECT_SOURCE_DIR}")

add_executable(${PROJECT_NAME} "main.cpp" "core/application.cpp" "core/application.h" "core/logging.h" "core/imgui_build.cpp" "core/layer.h" "core/image.cpp" "core/image.h" "rendering/renderer.cpp" "rendering/renderer.h" "rendering/camera.h" "rendering/camera.cpp" "core/input/input.h" "core/input/input.cpp" "core/input/key_codes.h" "rendering/render_layer.cpp" "rendering/render_layer.h")

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
