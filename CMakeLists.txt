cmake_minimum_required(VERSION 3.20)
project(Rounder)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/glfw)

set(PHYSICS_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/JoltPhysics")
set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)
add_subdirectory("${PHYSICS_REPO_ROOT}/Build" JoltPhysics_build)

set(ENGINE_SOURCES
        engine/vulkan_buffer.cpp
        engine/vulkan_context.cpp
        engine/vulkan_debug.cpp
        engine/vulkan_pipeline.cpp
        engine/vulkan_swapchain.cpp
        engine/vulkan_renderer.cpp
        engine/vulkan_texture.cpp
        engine/debug_ui.cpp
        engine/physics_world.cpp
        engine/scene_loader.cpp
)

set(GAME_SOURCES
        src/main.cpp
        src/game.cpp
        src/processInput.cpp
        src/character.cpp
        src/projectile_manager.cpp
)

set(THIRD_PARTY_SOURCES
        third_party/imgui/imgui.cpp
        third_party/imgui/imgui_draw.cpp
        third_party/imgui/imgui_tables.cpp
        third_party/imgui/imgui_widgets.cpp
        third_party/imgui/imgui_demo.cpp
        third_party/imgui/backends/imgui_impl_glfw.cpp
        third_party/imgui/backends/imgui_impl_vulkan.cpp
)

add_executable(${PROJECT_NAME} ${ENGINE_SOURCES} ${GAME_SOURCES} ${THIRD_PARTY_SOURCES})

target_link_libraries(${PROJECT_NAME}
        Vulkan::Vulkan
        glfw
        Jolt
)

target_include_directories(${PROJECT_NAME} PRIVATE
        "third_party/glm"
        "third_party/imgui"
        "third_party/imgui/backends"
        "third_party/stb"
        "third_party/tinyobjloader"
        "third_party/JoltPhysics"
        "third_party/json/include"
)