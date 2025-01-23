cmake_minimum_required(VERSION 3.31)

project(cg VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(external/imgui)
include_directories(external/imgui/backends)
include_directories(include)

file(GLOB IMGUI_SOURCE_FILES "external/imgui/*.cpp" "external/imgui/backends/imgui_impl_opengl3.cpp" "external/imgui/backends/imgui_impl_sdl2.cpp")
file(GLOB SOURCE_FILES "src/*.cpp")

add_executable(${PROJECT_NAME} ${IMGUI_SOURCE_FILES} ${SOURCE_FILES})

# Link system-wide libraries
find_package(assimp REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp)
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::OpenGL)
find_package(GLEW REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW)
find_package(glm REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
find_package(SDL2 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)
