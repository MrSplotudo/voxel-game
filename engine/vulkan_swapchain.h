#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanSwapchain {
public:
    VulkanSwapchain(VkPhysicalDevice physicalDeviceIn, VkDevice deviceIn, VkSurfaceKHR surfaceIn);
    ~VulkanSwapchain();
    void create(uint32_t widthIn, uint32_t heightIn);
    void cleanup();

    VkSwapchainKHR getHandle() const { return swapchain; }
    VkFormat getImageFormat() const { return swapchainImageFormat; }
    VkExtent2D getExtent() const { return swapchainExtent; }
    const std::vector<VkImage>& getImages() const { return swapchainImages; }
    const std::vector<VkImageView>& getImageViews() const { return swapchainImageViews; }
    VkImageView getDepthImageView() const { return depthImageView; }

private:
    void createSwapchain(uint32_t widthIn, uint32_t heightIn);
    void createImageViews();

    void createDepthResources();
    VkFormat findDepthFormat() const;
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    SwapchainSupportDetails querySwapchainSupport() const;
    static VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    static VkPresentModeKHR chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    static VkExtent2D chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t widthIn, uint32_t heightIn);

    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> swapchainImages = std::vector<VkImage>();
    VkFormat swapchainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D swapchainExtent = VkExtent2D();
    std::vector<VkImageView> swapchainImageViews = std::vector<VkImageView>();
    VkImage depthImage = VK_NULL_HANDLE;
    VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;

};