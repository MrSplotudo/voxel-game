cmake_minimum_required(VERSION 3.20)
project(VoxelGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

set(GLFW_DIR "${CMAKE_SOURCE_DIR}/../../01.03 - glfw_binaries/glfw-3.4.bin.WIN64")

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

add_executable(${PROJECT_NAME} ${ENGINE_SOURCES} ${GAME_SOURCES})

target_link_libraries(${PROJECT_NAME}
        Vulkan::Vulkan
        ${GLFW_DIR}/lib-vc2022/glfw3.lib

)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${GLFW_DIR}/include
        "${CMAKE_SOURCE_DIR}/../../01.02 - vulkan_sdk/Include"
)