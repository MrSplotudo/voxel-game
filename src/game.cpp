#include "game.h"
#include "../engine/vulkan_context.h"
#include "../engine/vulkan_swapchain.h"
#include "../engine/vulkan_pipeline.h"
#include "../engine/vulkan_renderer.h"
#include "../engine/vulkan_texture.h"
#include "../engine/vulkan_buffer.h"
#include "../engine/vulkan_vertex.h"
#include "../third_party/imgui/imgui.h"
#include "../third_party/imgui/backends/imgui_impl_vulkan.h"
#include "../third_party/imgui/backends/imgui_impl_glfw.h"
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

    dirtTexture = new VulkanTexture(
        vulkanContext->getDevice(),
        vulkanContext->getPhysicalDevice(),
        vulkanContext->getGraphicsQueue(),
        vulkanContext->findQueueFamilies(vulkanContext->getPhysicalDevice()).graphicsFamily);
    dirtTexture->load("../textures/dirt.png", vulkanPipeline->getDescriptorSetLayout());

    grassTexture = new VulkanTexture(
        vulkanContext->getDevice(),
        vulkanContext->getPhysicalDevice(),
        vulkanContext->getGraphicsQueue(),
        vulkanContext->findQueueFamilies(vulkanContext->getPhysicalDevice()).graphicsFamily);
    grassTexture->load("../textures/grass.png", vulkanPipeline->getDescriptorSetLayout());

    std::vector<Vertex> cubeVertices = {
        // Front face
        {{-0.5f, -0.5f,  -0.5f}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f,  -0.5f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f,  -0.5f}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f,  -0.5f}, {0.0f, 1.0f}},
        {{ 0.5f,  0.5f,  -0.5f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f,  -0.5f}, {0.0f, 0.0f}},
        // Back face
        {{ 0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, 0.5f}, {1.0f, 0.0f}},
        {{ 0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},
        {{-0.5f,  0.5f, 0.5f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, 0.5f}, {0.0f, 0.0f}},
        // Top face
        {{-0.5f,  0.5f,  -0.5f}, {0.0f, 1.0f}},
        {{ 0.5f,  0.5f,  -0.5f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f, 0.5f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f,  -0.5f}, {0.0f, 1.0f}},
        {{ 0.5f,  0.5f, 0.5f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f, 0.5f}, {0.0f, 0.0f}},
        // Bottom face
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}},
        {{ 0.5f, -0.5f,  -0.5f}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f,  -0.5f}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f,  -0.5f}, {0.0f, 0.0f}},
        // Right face
        {{ 0.5f, -0.5f,  -0.5f}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f, 0.5f}, {1.0f, 0.0f}},
        {{ 0.5f, -0.5f,  -0.5f}, {0.0f, 1.0f}},
        {{ 0.5f,  0.5f, 0.5f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f,  -0.5f}, {0.0f, 0.0f}},
        // Left face
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f,  -0.5f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f,  -0.5f}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},
        {{-0.5f,  0.5f,  -0.5f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f, 0.5f}, {0.0f, 0.0f}},
    };

    cubeMesh = new VulkanBuffer(vulkanContext->getDevice(), vulkanContext->getPhysicalDevice());
    cubeMesh->create(sizeof(Vertex) * cubeVertices.size(), cubeVertices.data());

    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            gameObjects.push_back({glm::vec3(i, 2.0f, j), cubeMesh, grassTexture});
            gameObjects.push_back({glm::vec3(i, 1.0f, j), cubeMesh, dirtTexture});
            gameObjects.push_back({glm::vec3(i, 0.0f, j), cubeMesh, dirtTexture});
        }
    }

    camera = new Camera(glm::vec3(0.0f, 5.0f, 3.0f));

    ImGui_ImplVulkan_InitInfo imGuiInitInfo = {};
    imGuiInitInfo.Instance = vulkanContext->getInstance();
    imGuiInitInfo.PhysicalDevice = vulkanContext->getPhysicalDevice();
    imGuiInitInfo.Device = vulkanContext->getDevice();
    imGuiInitInfo.QueueFamily = vulkanContext->findQueueFamilies(vulkanContext->getPhysicalDevice()).graphicsFamily;
    imGuiInitInfo.Queue = vulkanContext->getGraphicsQueue();
    imGuiInitInfo.PipelineInfoMain.RenderPass = vulkanPipeline->getRenderPass();
    imGuiInitInfo.ImageCount = vulkanSwapchain->getImages().size();
    imGuiInitInfo.MinImageCount = vulkanSwapchain->getImages().size();

    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 100;
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 100;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vkCreateDescriptorPool(vulkanContext->getDevice(), &poolInfo, nullptr, &imGuiDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
    imGuiInitInfo.DescriptorPool = imGuiDescriptorPool;

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_Init(&imGuiInitInfo);
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

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
        ImGui::Text("Objects: %d", (int)gameObjects.size());
        ImGui::Render();

        vulkanRenderer->drawObjects(gameObjects, camera->getViewMatrix());
    }

    vkDeviceWaitIdle(vulkanContext->getDevice());
}

void Game::cleanup() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(vulkanContext->getDevice(), imGuiDescriptorPool, nullptr);
    delete vulkanRenderer;
    delete vulkanPipeline;
    delete vulkanSwapchain;
    delete dirtTexture;
    delete grassTexture;
    delete cubeMesh;
    delete vulkanContext;
    glfwDestroyWindow(window);
    glfwTerminate();

    delete camera;
}
