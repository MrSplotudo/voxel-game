cmake_minimum_required(VERSION 3.20)
project(VoxelGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/glfw)

set(ENGINE_SOURCES
        engine/vulkan_buffer.cpp
        engine/vulkan_context.cpp
        engine/vulkan_debug.cpp
        engine/vulkan_pipeline.cpp
        engine/vulkan_swapchain.cpp
        engine/vulkan_renderer.cpp
        engine/vulkan_texture.cpp
)

set(GAME_SOURCES
        src/main.cpp
        src/game.cpp
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
)

target_include_directories(${PROJECT_NAME} PRIVATE
        "third_party/glm"
        "third_party/imgui"
        "third_party/imgui/backends"
        "third_party/stb"
)