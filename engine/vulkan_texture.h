#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

class VulkanContext;

class VulkanTexture {
public:
    VulkanTexture(VkDevice deviceIn, VkPhysicalDevice physicalDeviceIn);
    ~VulkanTexture();

    static void load(const std::string& filePath);

private:

    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
};