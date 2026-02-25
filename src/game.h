#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "game_object.h"
#include "projectile_manager.h"

class VulkanContext;
class VulkanSwapchain;
class VulkanPipeline;
class VulkanRenderer;
class VulkanTexture;
class VulkanBuffer;
class PhysicsWorld;
class SceneLoader;
class Character;
class ProjectileManager;
class Camera;
class ProcessInput;
class DebugUI;

class Game {
public:
    void run();

private:
    void initEngine();
    void initGame();
    void mainLoop();
    void cleanup();

    GLFWwindow* window = nullptr;
    VulkanContext* vulkanContext = nullptr;
    VulkanSwapchain* vulkanSwapchain = nullptr;
    VulkanPipeline* vulkanPipeline = nullptr;
    VulkanRenderer* vulkanRenderer = nullptr;
    DebugUI* debugUI = nullptr;

    PhysicsWorld* physicsWorld = nullptr;
    SceneLoader* sceneLoader = nullptr;

    std::vector<GameObject> gameObjects = {};
    Character* character = nullptr;
    ProjectileManager* projectileManager = nullptr;

    Camera* camera = nullptr;
    ProcessInput* processInput = nullptr;

    float lastFrame = 0.0f;
    float deltaTime = 0.0f;
    double lastMouseX = 400.0;
    double lastMouseY = 400.0;
    bool firstMouse = true;

    const uint32_t WIDTH = 2550;
    const uint32_t HEIGHT = 1440;
};