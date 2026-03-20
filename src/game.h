#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "game_object.h"
#include "visual_object.h"
#include "collision_zone.h"
#include "projectile_manager.h"
#include "palette_entry.h"
#include "../engine/lighting_ubo.h"

class VulkanContext;
class VulkanSwapchain;
class VulkanPipeline;
class VulkanRenderer;
class VulkanTexture;
class VulkanBuffer;
class PhysicsWorld;
class AssetCache;
class SceneLoader;
class Character;
class ProjectileManager;
class Camera;
class ProcessInput;
class DebugUI;

enum class EditorTool {
    PlaceVisual,
    DragCollisionZone,
    SelectElement
};

enum class DragState {
    Idle,
    Dragging
};

class Game {
    uint32_t width = 2550;
    uint32_t height = 1440;

public:
    void run();
    bool framebufferResized = false;
private:
    void initEngine();
    void initGame();
    void mainLoop();
    void cleanup();
    void updatePlayMode(float deltaTime);
    void updateEditorMode(float deltaTime);
    void recreateSwapchain();

    GLFWwindow* window = nullptr;
    VulkanContext* vulkanContext = nullptr;
    VulkanSwapchain* vulkanSwapchain = nullptr;
    VulkanPipeline* vulkanPipeline = nullptr;
    VulkanRenderer* vulkanRenderer = nullptr;
    DebugUI* debugUI = nullptr;

    PhysicsWorld* physicsWorld = nullptr;
    AssetCache* assetCache = nullptr;
    SceneLoader* sceneLoader = nullptr;

    std::vector<PaletteEntry> palette = {
        {"Stone Platform", "assets/models/platform.obj", "assets/textures/platform_texture.png"},
    };

    std::vector<GameObject> gameObjects = {};
    std::vector<VisualObject> visualObjects = {};
    std::vector<CollisionZone> collisionZones = {};

    LightingUBO lightingData = {};

    Character* character = nullptr;
    ProjectileManager* projectileManager = nullptr;
    Camera* camera = nullptr;
    ProcessInput* processInput = nullptr;

    float lastFrame = 0.0f;
    float deltaTime = 0.0f;
    double lastMouseX = 400.0;
    double lastMouseY = 400.0;
    bool firstMouse = true;

    bool editMode = false;
    bool drawHitboxes = false;
    std::string levelName = "";

    EditorTool currentTool = EditorTool::PlaceVisual;
    DragState dragState = DragState::Idle;
    glm::vec3 dragStart = {};
    int paletteIndex = -1;
    float gridSize = 1.0f;

    float cameraOffsetY = 0.05f;
    float cameraOffsetZ = 9.0f;
};

