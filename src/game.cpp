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
#include "../engine/asset_cache.h"
#include "../engine/scene_loader.h"
#include <iostream>

void framebufferSizeCallback(GLFWwindow* w, int width, int height) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(w));
    game->framebufferResized = true;
}

void Game::run() {
    initEngine();
    initGame();
    mainLoop();
    cleanup();
}

void Game::cleanup() {
    delete debugUI;
    delete character;
    projectileManager->shutdown();
    delete projectileManager;
    delete processInput;
    delete assetCache;
    delete sceneLoader;
    delete physicsWorld;
    delete vulkanRenderer;
    delete vulkanPipeline;
    delete vulkanSwapchain;
    delete vulkanContext;
    glfwDestroyWindow(window);
    glfwTerminate();
    delete camera;
}

void Game::initEngine() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width, height, "Vulkan App", nullptr, nullptr);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetWindowUserPointer(window, this);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    vulkanContext = new VulkanContext(window);

    vulkanSwapchain = new VulkanSwapchain(vulkanContext->getPhysicalDevice(), vulkanContext->getDevice(), vulkanContext->getSurface());
    vulkanSwapchain->create(width, height);

    vulkanPipeline = new VulkanPipeline(vulkanContext->getDevice(),vulkanSwapchain->getImageFormat(),vulkanSwapchain->getExtent());
    vulkanPipeline->createPipeline();

    vulkanRenderer = new VulkanRenderer(vulkanContext, vulkanSwapchain, vulkanPipeline, width, height);
    vulkanRenderer->create();

    physicsWorld = new PhysicsWorld();
    physicsWorld->create();

    assetCache = new AssetCache(vulkanContext, vulkanPipeline);
    sceneLoader = new SceneLoader(vulkanContext, vulkanPipeline, physicsWorld, assetCache);

    physicsWorld->getContactListener()->onContact = [](const JPH::Body& body1, const JPH::Body& body2) {
        if (body1.GetUserData()) {
            Projectile* projectile = reinterpret_cast<Projectile*>(body1.GetUserData());
            projectile->properties.bouncesRemaining--;
        }
        if (body2.GetUserData()) {
            Projectile* projectile = reinterpret_cast<Projectile*>(body2.GetUserData());
            projectile->properties.bouncesRemaining--;
        }
    };

    debugUI = new DebugUI(window, vulkanContext->getInstance(), vulkanContext->getPhysicalDevice(), vulkanContext->getDevice(), vulkanContext->getGraphicsQueue(), vulkanPipeline->getRenderPass(), vulkanSwapchain->getImages().size(), vulkanContext->findQueueFamilies(vulkanContext->getPhysicalDevice()).graphicsFamily);
    debugUI->create();

    processInput = new ProcessInput(window, physicsWorld->getBodyInterface());
}

void Game::initGame() {
    sceneLoader->load("../levels/lab.json", gameObjects, visualObjects, collisionZones);

    projectileManager = new ProjectileManager(physicsWorld, vulkanContext, vulkanPipeline);
    projectileManager->init("../assets/models/bullet.obj", "../assets/textures/bullet_texture.png");

    character = new Character(vulkanContext, vulkanPipeline, physicsWorld, assetCache, projectileManager);
    character->spawn({0.0f, 20.0f, 0.0f});


    camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f));

    lightingData.lightDirection = glm::vec3(0.3f, 1.0f, 0.5f);
    lightingData.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    lightingData.cameraPosition = camera->position;
    lightingData.ambientStrength = 0.15f;
    lightingData.specularStrength = 0.5f;
    lightingData.shininess = 0.32f;
}
void Game::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (deltaTime > 0.05f) deltaTime = 0.05f;

        glfwPollEvents();

        if (framebufferResized) {
            recreateSwapchain();
        }

        glm::mat4 projection = glm::perspective(
                glm::radians(85.0f),
                static_cast<float>(width) / static_cast<float>(height),
                0.1f, 100.0f);
        projection[1][1] *= -1;  // Vulkan Y-flip, must match renderer
        glm::mat4 vp = projection * camera->getViewMatrix();

        if (!editMode) {
            updatePlayMode(deltaTime);
        }
        else if (editMode) {
            updateEditorMode(deltaTime);
        }

        // Imgui
        debugUI->beginFrame();
        ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
        ImGui::Text("Objects: %d", (int)gameObjects.size() + (int)projectileManager->getProjectiles().size());
        ImGui::SliderFloat("Ambient strength", &lightingData.ambientStrength, 0.0f, 1.0f);
        ImGui::SliderFloat("Specular strength", &lightingData.specularStrength, 0.0f, 1.0f);
        ImGui::SliderFloat("Shininess", &lightingData.shininess, 0.0f, 1.0f);
        ImGui::DragFloat("Camera offset Y", &cameraOffsetY, 0.001f);
        ImGui::DragFloat("Camera offset Z", &cameraOffsetZ, 0.001f);
        ImGui::DragFloat("FireCooldown", &character->fireCooldown, 0.001f);
        ImGui::DragFloat("BulletSpeed", &character->bulletSpeed, 0.001f);
        ImGui::DragFloat("BulletLifeTime", &character->bulletLifeTime, 0.001f);
        ImGui::DragFloat("BulletGravity", &character->bulletGravity, 0.001f);
        ImGui::DragInt("Bounces", &character->bounces, 1);
        ImGui::DragFloat("BulletBounciness", &character->bulletBounciness, 0.001f);
        ImGui::Checkbox("Draw Hitboxes", &drawHitboxes);
        ImGui::Checkbox("EditMode", &editMode);
        if (drawHitboxes) {
            debugUI->renderHitboxes(vp, width, height, gameObjects, collisionZones);
        }

        if (editMode) {
            ImGui::Begin("Assets Window");

            ImGui::DragFloat("Grid Size", &gridSize, 0.1f, 0.1f, 10.0f);
            if (ImGui::RadioButton("Draw Collision", currentTool == EditorTool::DragCollisionZone))
                currentTool = EditorTool::DragCollisionZone;
            if (ImGui::RadioButton("Place Visual", currentTool == EditorTool::PlaceVisual))
                currentTool = EditorTool::PlaceVisual;
            if (ImGui::RadioButton("Select Element", currentTool == EditorTool::SelectElement))
                currentTool = EditorTool::SelectElement;

            for (int i = 0; i < palette.size(); i++) {
                bool isSelected = (paletteIndex == i);
                if (ImGui::Selectable(palette[i].name.c_str(), isSelected)) {
                    paletteIndex = i;
                }
            }

            if (ImGui::Button("Save")) {
                sceneLoader->save("../levels/", "lab", gameObjects, visualObjects, collisionZones);
            }

            ImGui::End();
        }
        ImGui::Begin("Character Movement");
        ImGui::DragFloat("Max Speed", &character->maxSpeed, 0.001f);
        ImGui::DragFloat("Acceleration", &character->acceleration, 0.001f);
        ImGui::DragFloat("Friction", &character->friction, 0.001f);
        ImGui::DragFloat("Jump Force", &character->jumpForce, 0.001f);
        ImGui::DragInt("Jumps", &character->jumps);
        ImGui::DragFloat("Fall Multiplier", &character->fallMultiplier, 0.001f);
        ImGui::DragFloat("rise Multiplier", &character->riseMultiplier, 0.001f);
        ImGui::DragFloat("Jump Cut Multiplier", &character->jumpCutMultiplier, 0.001f);
        ImGui::End();



        debugUI->endFrame();

        std::vector<GameObject> objectsToDraw = gameObjects;
        objectsToDraw.push_back(character->object);
        objectsToDraw.push_back(character->gun);

        lightingData.cameraPosition = camera->position;

        vulkanRenderer->updateLighting(lightingData);
        vulkanRenderer->drawObjects(objectsToDraw, visualObjects, projectileManager->getProjectiles(), camera->getViewMatrix());
    }

    vkDeviceWaitIdle(vulkanContext->getDevice());
}

void Game::updatePlayMode(float deltaTime) {
    InputState input = processInput->getInputState();
    glm::vec3 crosshairWorldPos = processInput->getWorldCursorPos(camera->getViewMatrix(), glm::perspective(glm::radians(85.0f), static_cast<float>(width) / height, 0.1f, 100.0f), width, height);

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

    camera->position = glm::vec3(playerPos.x, playerPos.y + cameraOffsetY, playerPos.z + cameraOffsetZ);

    glm::vec3 aimOrigin = playerPos + character->gunOffset;
    glm::vec3 toTarget = crosshairWorldPos - aimOrigin;
    glm::vec3 fireDirection = glm::normalize(toTarget);
    fireDirection.z = 0.0f; // enforce 2D

    bool mouseOnUI = ImGui::GetIO().WantCaptureMouse;
    if (mouseOnUI){
        input.shoot = false;
    }



    physicsWorld->update(deltaTime);
    character->update(deltaTime, input);
    character->updateGun(fireDirection);
    projectileManager->update(deltaTime);
}

void Game::updateEditorMode(float deltaTime) {
    glm::vec3 cursorWorldPos = processInput->getWorldCursorPos(camera->getViewMatrix(), glm::perspective(glm::radians(85.0f), static_cast<float>(width) / height, 0.1f, 100.0f), width, height);
    cursorWorldPos.z = 0.0f;

    bool mouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool mouseOnUI = ImGui::GetIO().WantCaptureMouse;

    if (!mouseOnUI){
        switch (currentTool) {
        case EditorTool::PlaceVisual :
            if (gridSize > 0.0f) {
                cursorWorldPos.x = round(cursorWorldPos.x / gridSize) * gridSize;
                cursorWorldPos.y = round(cursorWorldPos.y / gridSize) * gridSize;
            }
            if (mouseDown && dragState == DragState::Idle && paletteIndex >= 0) {
                dragState = DragState::Dragging;

                PaletteEntry& entry = palette[paletteIndex];
                MeshBuffers meshBuffers = assetCache->getMesh(entry.meshPath);
                VulkanTexture* texture = assetCache->getTexture(entry.texturePath);

                VisualObject newObject;
                newObject.transform.position = cursorWorldPos;
                newObject.mesh = meshBuffers.vertexBuffer;
                newObject.indexBuffer = meshBuffers.indexBuffer;
                newObject.texture = texture;
                newObject.meshPath = entry.meshPath;
                newObject.texturePath = entry.texturePath;
                visualObjects.push_back(newObject);
            }
            else if (!mouseDown) {
                dragState = DragState::Idle;
            }

            break;

        case EditorTool::DragCollisionZone :
            if (gridSize > 0.0f) {
                float offset = 0.5f;
                cursorWorldPos.x = round((cursorWorldPos.x - offset) / gridSize) * gridSize + offset;
                cursorWorldPos.y = round((cursorWorldPos.y - offset) / gridSize) * gridSize + offset;
            }
            if (mouseDown && dragState == DragState::Idle) {
                dragState = DragState::Dragging;
                dragStart = cursorWorldPos;
            }
            if (!mouseDown && dragState == DragState::Dragging) {
                dragState = DragState::Idle;

                glm::vec3 halfExtents = glm::abs(cursorWorldPos - dragStart) / 2.0f;
                halfExtents.z = 0.5f;
                glm::vec3 center = (dragStart + cursorWorldPos) / 2.0f;

                JPH::BodyID bodyID = physicsWorld->createBody(
                                                              JPH::Vec3(center.x, center.y, center.z),
                                                              JPH::Vec3(halfExtents.x, halfExtents.y, halfExtents.z),
                                                              JPH::EMotionType::Static,
                                                              Layers::STATIC);

                collisionZones.push_back({center, halfExtents, bodyID});
            }
            break;

        case EditorTool::SelectElement :
            if (mouseDown && dragState == DragState::Idle) {
                dragState = DragState::Dragging;

                float offset = 0.5f;
                float smallestDistance = offset;
                int chosenIndex = -1;

                for (int i = 0; i < visualObjects.size(); i++) {
                    float distance = glm::length(cursorWorldPos - visualObjects[i].transform.position);

                    if (distance < smallestDistance) {
                        chosenIndex = i;
                        smallestDistance = distance;
                    }
                }
                if (smallestDistance < offset && chosenIndex >= 0) {
                    visualObjects.erase(visualObjects.begin() + chosenIndex);
                    break;
                }

                for (int i = 0; i < collisionZones.size(); i++) {
                    float distance = glm::length(cursorWorldPos - collisionZones[i].position);
                    if (distance < smallestDistance) {
                        chosenIndex = i;
                        smallestDistance = distance;
                    }
                }
                if (smallestDistance < offset && chosenIndex >= 0) {
                    physicsWorld->getBodyInterface()->RemoveBody(collisionZones[chosenIndex].bodyID);
                    physicsWorld->getBodyInterface()->DestroyBody(collisionZones[chosenIndex].bodyID);
                    collisionZones.erase(collisionZones.begin() + chosenIndex);
                    break;
                }
            }
            if (!mouseDown && dragState == DragState::Dragging) {
                dragState = DragState::Idle;
                break;
            }
            break;

        default: ;
        }
    }

    float camSpeed = 5.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera->position.y += camSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera->position.y -= camSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera->position.x += camSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera->position.x -= camSpeed * deltaTime;
}

void Game::recreateSwapchain() {
    // Handle minimization — wait until the window has real dimensions
    int w = 0, h = 0;
    glfwGetFramebufferSize(window, &w, &h);
    while (w == 0 || h == 0) {
        glfwGetFramebufferSize(window, &w, &h);
        glfwWaitEvents();
    }

    // Wait for GPU to finish all work
    vkDeviceWaitIdle(vulkanContext->getDevice());

    // Destroy in dependency order
    vulkanRenderer->destroyFramebuffers();
    vulkanSwapchain->cleanup();

    // Update stored dimensions
    width = static_cast<uint32_t>(w);
    height = static_cast<uint32_t>(h);

    // Recreate in reverse order
    vulkanSwapchain->create(width, height);
    vulkanRenderer->recreateFramebuffers();

    framebufferResized = false;
}