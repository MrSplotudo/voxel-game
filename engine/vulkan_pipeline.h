#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>


class VulkanPipeline {
public:
    VulkanPipeline(VkDevice deviceIn, VkFormat formatIn, VkExtent2D extentIn);
    ~VulkanPipeline();

    void createPipeline();

    VkRenderPass getRenderPass() const { return renderPass; }
    VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
    VkPipeline getGraphicsPipeline() const { return graphicsPipeline; }
    VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
    VkDescriptorSetLayout getUBODescriptorSetLayout() const { return uboDescriptorSetLayout; }

private:
    void createRenderPass();
    VkShaderModule createShaderModule(const std::vector<char>& code) const;
    void createGraphicsPipeline();
    void createDescriptorSetLayout();
    void createUBODescriptorSetLayout();

    VkDevice device;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout uboDescriptorSetLayout = VK_NULL_HANDLE;
};