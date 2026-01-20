#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

class VulkanContext;
class VulkanSwapchain;
class VulkanPipeline;
class VulkanBuffer;

class VulkanRenderer {
public:
    VulkanRenderer(VulkanContext* contextIn, VulkanSwapchain* swapchainIn, VulkanPipeline* pipelineIn, uint32_t widthIn, uint32_t heightIn);
    ~VulkanRenderer();

    void create();
    void drawFrame(const glm::mat4& viewMatrix);
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const glm::mat4& viewMatrix);

private:
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();

    VulkanContext* context;
    VulkanSwapchain* swapchain;
    VulkanPipeline* pipeline;
    VulkanBuffer* vertexBuffer = nullptr;

    std::vector <VkFramebuffer> swapchainFramebuffers;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    uint32_t height;
    uint32_t width;

    const int MAX_FRAMES_IN_FLIGHT = 2;
    size_t currentFrame = 0;
};