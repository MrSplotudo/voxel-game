#include "game.h"
#include "camera.h"
#include "processInput.h"
#include "character.h"
#include "projectile_manager.h"
#include "input_state.h"
#include "../engine/vulkan_context.h"
#include "../engine/vulkan_swapchain.h"
#include "../engine/vulkan_pipeline.h"
#include "../engine/vulkan_renderer.h"
#include "../engine/vulkan_buffer.h"
#include "../engine/vulkan_vertex.h"
#include "../engine/physics_world.h"
#include "../engine/debug_ui.h"
#include "../engine/scene_loader.h"
#include <iostream>

void Game::run() {
    initEngine();
    initGame();
    mainLoop();
    cleanup();
}

void Game::initEngine() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan App", nullptr, nullptr);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    vulkanContext = new VulkanContext(window);

    vulkanSwapchain = new VulkanSwapchain(vulkanContext->getPhysicalDevice(), vulkanContext->getDevice(), vulkanContext->getSurface());
    vulkanSwapchain->create(WIDTH, HEIGHT);

    vulkanPipeline = new VulkanPipeline(vulkanContext->getDevice(),vulkanSwapchain->getImageFormat(),vulkanSwapchain->getExtent());
    vulkanPipeline->createPipeline();

    vulkanRenderer = new VulkanRenderer(vulkanContext, vulkanSwapchain, vulkanPipeline, WIDTH, HEIGHT);
    vulkanRenderer->create();

    physicsWorld = new PhysicsWorld();
    physicsWorld->create();

    sceneLoader = new SceneLoader(vulkanContext, vulkanPipeline, physicsWorld);

    debugUI = new DebugUI(window, vulkanContext->getInstance(), vulkanContext->getPhysicalDevice(), vulkanContext->getDevice(), vulkanContext->getGraphicsQueue(), vulkanPipeline->getRenderPass(), vulkanSwapchain->getImages().size(), vulkanContext->findQueueFamilies(vulkanContext->getPhysicalDevice()).graphicsFamily);
    debugUI->create();

    processInput = new ProcessInput(window, physicsWorld->getBodyInterface());
}

void Game::initGame() {
    sceneLoader->load("../levels/lab.json", gameObjects);

    character = new Character(vulkanContext, vulkanPipeline, physicsWorld);
    character->spawn({0.0f, 20.0f, 0.0f});

    projectileManager = new ProjectileManager(physicsWorld, vulkanContext, vulkanPipeline);
    projectileManager->init("../assets/models/bullet.obj", "../assets/textures/bullet_texture.png");

    camera = new Camera(glm::vec3(0.0f, 3.0f, 9.0f));
}
void Game::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        InputState input = processInput->getInputState();

        // Sync object positions
        for (auto& object : gameObjects) {
            if (object.bodyID.IsInvalid()) {
                continue;
            }
            JPH::Vec3 physPos = physicsWorld->getPosition(object.bodyID);
            JPH::Quat physRot = physicsWorld->getRotation(object.bodyID);

            object.transform.position = glm::vec3(physPos.GetX(), physPos.GetY(), physPos.GetZ());
            object.transform.rotation = glm::quat(physRot.GetW(), physRot.GetX(), physRot.GetY(), physRot.GetZ());
        }

        // Sync character position
        JPH::Vec3 physPos = physicsWorld->getPosition(character->object.bodyID);
        JPH::Quat physRot = physicsWorld->getRotation(character->object.bodyID);
        character->object.transform.position = glm::vec3(physPos.GetX(), physPos.GetY(), physPos.GetZ());
        character->object.transform.rotation = glm::quat(physRot.GetW(), physRot.GetX(), physRot.GetY(), physRot.GetZ());
        glm::vec3 playerPos = character->object.transform.position;

        glm::vec3 crosshairWorldPos = processInput->getWorldCursorPos(camera->getViewMatrix(), glm::perspective(glm::radians(85.0f), static_cast<float>(WIDTH) / HEIGHT, 0.1f, 100.0f), WIDTH, HEIGHT);

        glm::vec3 aimOrigin = playerPos + character->gunOffset;
        glm::vec3 toTarget = crosshairWorldPos - aimOrigin;
        glm::vec3 fireDirection = glm::normalize(toTarget);
        fireDirection.z = 0.0f; // enforce 2D

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            ProjectileProperties props;
            props.speed   = 5.0f;
            props.lifespan = 10.0f;

            glm::vec3 spawnPos = character->getBarrelTip();
            projectileManager->spawn(spawnPos, fireDirection, props);
        }

        physicsWorld->update(deltaTime);
        character->update(deltaTime, input);
        character->updateGun(fireDirection);
        projectileManager->update(deltaTime);

        debugUI->beginFrame();
        ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
        ImGui::Text("Objects: %d", (int)gameObjects.size());
        debugUI->endFrame();

        std::vector<GameObject> objectsToDraw = gameObjects;
        objectsToDraw.push_back(character->object);
        objectsToDraw.push_back(character->gun);

        vulkanRenderer->drawObjects(objectsToDraw, projectileManager->getProjectiles(), camera->getViewMatrix());
    }

    vkDeviceWaitIdle(vulkanContext->getDevice());
}

void Game::cleanup() {
    delete debugUI;
    delete vulkanRenderer;
    delete vulkanPipeline;
    delete vulkanSwapchain;
    for (auto& object : gameObjects) {
        delete object.mesh;
        delete object.indexBuffer;
        delete object.texture;
    }
    projectileManager->shutdown();
    delete projectileManager;
    delete vulkanContext;
    glfwDestroyWindow(window);
    glfwTerminate();
    delete camera;
}
