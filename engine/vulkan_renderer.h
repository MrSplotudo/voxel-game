#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <deque>
#include "lighting_ubo.h"
#include "../src/game_object.h"
#include "../src/visual_object.h"
#include "../src/projectile.h"


class VulkanContext;
class VulkanSwapchain;
class VulkanPipeline;
class VulkanBuffer;

class VulkanRenderer {
public:
    VulkanRenderer(VulkanContext* contextIn, VulkanSwapchain* swapchainIn, VulkanPipeline* pipelineIn, uint32_t widthIn, uint32_t heightIn);
    ~VulkanRenderer();

    void create();
    void drawObjects(const std::vector<GameObject>& objects, const std::vector<VisualObject>& visualObjects, const std::deque<std::unique_ptr<Projectile>>& projectiles, const glm::mat4& viewMatrix);

    void updateLighting(const LightingUBO& data);
    void destroyFramebuffers();
    void recreateFramebuffers();

private:
    void createUBOResources();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const std::vector<GameObject>& objects,
        const std::vector<VisualObject>& visualObjects, const std::deque<std::unique_ptr<Projectile>>& projectiles, const glm::mat4& viewMatrix);

    VulkanContext* context;
    VulkanSwapchain* swapchain;
    VulkanPipeline* pipeline;

    std::vector <VkFramebuffer> swapchainFramebuffers;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    std::vector<VkBuffer> uboBuffers;
    std::vector<VkDeviceMemory> uboMemory;
    std::vector<void*> uboMapped;
    std::vector<VkDescriptorSet> uboDescriptorSets;
    VkDescriptorPool uboDescriptorPool = VK_NULL_HANDLE;

    uint32_t height;
    uint32_t width;

    const int MAX_FRAMES_IN_FLIGHT = 2;
    size_t currentFrame = 0;
};