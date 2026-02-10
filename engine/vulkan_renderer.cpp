#include "vulkan_renderer.h"
#include "vulkan_context.h"
#include "vulkan_swapchain.h"
#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"
#include "vulkan_vertex.h"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui_impl_vulkan.h"
#include <stdexcept>
#include <array>

VulkanRenderer::VulkanRenderer(VulkanContext* contextIn, VulkanSwapchain* swapchainIn, VulkanPipeline* pipelineIn, uint32_t widthIn, uint32_t heightIn) : context(contextIn), swapchain(swapchainIn), pipeline(pipelineIn), width(widthIn), height(heightIn) {
}

VulkanRenderer::~VulkanRenderer() {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyFence(context->getDevice(), inFlightFences[i], nullptr);
        vkDestroySemaphore(context->getDevice(), renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(context->getDevice(), imageAvailableSemaphores[i], nullptr);
    }

    vkDestroyCommandPool(context->getDevice(), commandPool, nullptr);

    for (size_t i = 0; i < swapchain->getImageViews().size(); i++) {
        vkDestroyFramebuffer(context->getDevice(), swapchainFramebuffers[i], nullptr);
    }
}

void VulkanRenderer::create() {
    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();
}

void VulkanRenderer::drawObjects(const std::vector<GameObject>& objects, const glm::mat4& viewMatrix) {
    vkWaitForFences(context->getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(context->getDevice(), 1, &inFlightFences[currentFrame]);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(context->getDevice(), swapchain->getHandle(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    vkResetCommandBuffer(commandBuffers[imageIndex], 0);
    recordCommandBuffer(commandBuffers[imageIndex], imageIndex, objects, viewMatrix);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(context->getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = {swapchain->getHandle()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(context->getGraphicsQueue(), &presentInfo);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const std::vector<GameObject>& objects, const glm::mat4& viewMatrix) {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin command buffer recording!");
    }

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = pipeline->getRenderPass();
    renderPassBeginInfo.framebuffer = swapchainFramebuffers[imageIndex];
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = swapchain->getExtent();

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = {{0.1f, 0.1f, 0.2f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getGraphicsPipeline());

    glm::mat4 projection = glm::perspective(
        glm::radians(85.0f),
        static_cast<float>(width) / static_cast<float>(height),
        0.1f,
        100.0f);

    projection[1][1] *= -1;

    for (const GameObject& object : objects) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), object.position);

        glm::mat4 mvp = projection * viewMatrix * model;

        vkCmdPushConstants(
        commandBuffer,
        pipeline->getPipelineLayout(),
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(mvp),
        &mvp);

        VkDescriptorSet descriptorSets[] = {object.texture->getDescriptorSet()};
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayout(), 0, 1, descriptorSets, 0, nullptr);

        VkBuffer buffers[] = {object.mesh->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        vkCmdDraw(commandBuffer, object.mesh->getVertexCount(), 1, 0, 0);
    }
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to end command buffer!");
    }
}

void VulkanRenderer::createFramebuffers() {
    swapchainFramebuffers.resize(swapchain->getImageViews().size());

    for (size_t i = 0; i < swapchain->getImageViews().size(); i++) {
        std::array<VkImageView, 2> attachments = {
            swapchain->getImageViews()[i],
            swapchain->getDepthImageView()
        };

        VkFramebufferCreateInfo framebufferCreateInfo = {};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = pipeline->getRenderPass();
        framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferCreateInfo.pAttachments = attachments.data();
        framebufferCreateInfo.width = swapchain->getExtent().width;
        framebufferCreateInfo.height = swapchain->getExtent().height;
        framebufferCreateInfo.layers = 1;

        if (vkCreateFramebuffer(context->getDevice(), &framebufferCreateInfo, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }
}

void VulkanRenderer::createCommandPool() {
    QueueFamilyIndices indices = context->findQueueFamilies(context->getPhysicalDevice());

    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = indices.graphicsFamily;

    if (vkCreateCommandPool(context->getDevice(), &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void VulkanRenderer::createCommandBuffers() {
    commandBuffers.resize(swapchainFramebuffers.size());

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(context->getDevice(), &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
}

void VulkanRenderer::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(context->getDevice(), &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(context->getDevice(), &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(context->getDevice(), &fenceCreateInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create synchronization objects!");
        }
    }
}