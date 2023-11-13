cmake_minimum_required(VERSION 3.16)

# create sample app project
project(siho LANGUAGES C CXX)

add_executable(${PROJECT_NAME} WIN32 "main.cpp" "core/application.cpp" "core/application.h" "core/logging.h" "core/imgui_build.cpp" "core/layer.h")

# Link third party libraries
target_link_libraries(${PROJECT_NAME} PUBLIC
    volk
    stb
    imgui
    tinygltf
    spdlog
    ctpl
    glfw)
