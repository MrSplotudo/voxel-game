#include "game.h"
#include "../engine/vulkan_context.h"
#include "../engine/vulkan_swapchain.h"
#include "../engine/vulkan_pipeline.h"
#include "../engine/vulkan_renderer.h"
#include "../engine/vulkan_texture.h"
#include <iostream>
#include <string>

void Game::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void Game::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan App", nullptr, nullptr);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Game::initVulkan() {
    vulkanContext = new VulkanContext(window);

    vulkanSwapchain = new VulkanSwapchain(
        vulkanContext->getPhysicalDevice(),
        vulkanContext->getDevice(),
        vulkanContext->getSurface());
    vulkanSwapchain->create(WIDTH, HEIGHT);

    vulkanPipeline = new VulkanPipeline(
        vulkanContext->getDevice(),
        vulkanSwapchain->getImageFormat(),
        vulkanSwapchain->getExtent());
    vulkanPipeline->createPipeline();

    vulkanRenderer = new VulkanRenderer(vulkanContext, vulkanSwapchain, vulkanPipeline, WIDTH, HEIGHT);
    vulkanRenderer->create();

    vulkanTexture = new VulkanTexture(vulkanContext->getDevice(), vulkanContext->getPhysicalDevice());
    vulkanTexture->load("../textures/dirt.png");

    camera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));
}

void Game::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();

        // Mouse input
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        if (firstMouse) {
            lastMouseX = mouseX;
            lastMouseY = mouseY;
            firstMouse = false;
        }
        camera->processMouse(mouseX - lastMouseX, mouseY - lastMouseY);
        lastMouseX = mouseX;
        lastMouseY = mouseY;

        // Keyboard input
        camera->processKeyboard(window, deltaTime);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        vulkanRenderer->drawFrame(camera->getViewMatrix());
    }

    vkDeviceWaitIdle(vulkanContext->getDevice());
}

void Game::cleanup() {
    delete vulkanRenderer;
    delete vulkanPipeline;
    delete vulkanSwapchain;
    delete vulkanContext;
    glfwDestroyWindow(window);
    glfwTerminate();

    delete camera;
}
