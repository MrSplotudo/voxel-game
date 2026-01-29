#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "camera.h"

class VulkanContext;
class VulkanSwapchain;
class VulkanPipeline;
class VulkanRenderer;
class VulkanTexture;


class Game {
public:
    void run();

private:
    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

    GLFWwindow* window = nullptr;
    VulkanContext* vulkanContext = nullptr;
    VulkanSwapchain* vulkanSwapchain = nullptr;
    VulkanPipeline* vulkanPipeline = nullptr;
    VulkanRenderer* vulkanRenderer = nullptr;
    VulkanTexture* vulkanTexture = nullptr;

    Camera* camera = nullptr;
    float lastFrame = 0.0f;
    float deltaTime = 0.0f;
    double lastMouseX = 400.0;
    double lastMouseY = 400.0;
    bool firstMouse = true;

    const uint32_t WIDTH = 1920;
    const uint32_t HEIGHT = 1080;
};