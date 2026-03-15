#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

class VulkanContext;

class VulkanTexture {
public:
    VulkanTexture(VkDevice deviceIn, VkPhysicalDevice physicalDeviceIn, VkQueue graphicsQueueIn, uint32_t queueFamilyIndexIn);
    ~VulkanTexture();

    void load(const std::string& filePath, VkDescriptorSetLayout descriptorSetLayout);

    VkDescriptorSet getDescriptorSet() const { return descriptorSet; }

private:
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
    void copyBufferToImage();
    void createImageView();
    void createSampler();
    void createDescriptorSet(VkDescriptorSetLayout descriptorSetLayout);

    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkQueue graphicsQueue;
    uint32_t queueFamilyIndex;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
    VkDeviceSize imageSize = 0;

    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    int texWidth = 0;
    int texHeight = 0;
    uint32_t mipLevels = 1;
};