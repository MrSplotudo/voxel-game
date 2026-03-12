#include "vulkan_renderer.h"
#include "vulkan_context.h"
#include "vulkan_swapchain.h"
#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"
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

        vkDestroyBuffer(context->getDevice(), uboBuffers[i], nullptr);
        vkFreeMemory(context->getDevice(), uboMemory[i], nullptr);
    }
    vkDestroyDescriptorPool(context->getDevice(), uboDescriptorPool, nullptr);

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
    createUBOResources();
}

void VulkanRenderer::drawObjects(const std::vector<GameObject>& objects, const std::vector<VisualObject>& visualObjects, const std::vector<Projectile>& projectiles, const glm::mat4& viewMatrix) {
    vkWaitForFences(context->getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(context->getDevice(), 1, &inFlightFences[currentFrame]);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(context->getDevice(), swapchain->getHandle(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    vkResetCommandBuffer(commandBuffers[imageIndex], 0);
    recordCommandBuffer(commandBuffers[imageIndex], imageIndex, objects, visualObjects, projectiles, viewMatrix);

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

void VulkanRenderer::updateLighting(const LightingUBO& data) {
    memcpy(uboMapped[currentFrame], &data, sizeof(LightingUBO));
}

void VulkanRenderer::createUBOResources() {
    VkDeviceSize bufferSize = sizeof(LightingUBO);

    uboBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uboMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uboMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size  = bufferSize;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(context->getDevice(), &bufferCreateInfo, nullptr, &uboBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create uniform buffer(UBO)!");
        }

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(context->getDevice(), uboBuffers[i], &memoryRequirements);

        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(context->getPhysicalDevice(), &memoryProperties);

        uint32_t memoryTypeIndex = 0;
        for (uint32_t j = 0; j < memoryProperties.memoryTypeCount; j++) {
            if ((memoryRequirements.memoryTypeBits & (1 << j)) &&
                (memoryProperties.memoryTypes[j].propertyFlags &
                (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) ==
                (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
                memoryTypeIndex = j;
                break;
            }
        }

        VkMemoryAllocateInfo memoryAllocateInfo = {};
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.allocationSize = memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

        if (vkAllocateMemory(context->getDevice(), &memoryAllocateInfo, nullptr, &uboMemory[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate UBO memory!");
        }

        vkBindBufferMemory(context->getDevice(), uboBuffers[i], uboMemory[i], 0);
        vkMapMemory(context->getDevice(), uboMemory[i], 0, bufferSize, 0, &uboMapped[i]);
    }

    VkDescriptorPoolSize descriptorPoolSize = {};
    descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &descriptorPoolSize;
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(context->getDevice(), &poolInfo, nullptr, &uboDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create UBO descriptor pool!");
    }

    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, pipeline->getUBODescriptorSetLayout());

    VkDescriptorSetAllocateInfo setAllocInfo = {};
    setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocInfo.descriptorPool = uboDescriptorPool;
    setAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    setAllocInfo.pSetLayouts = layouts.data();

    uboDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(context->getDevice(), &setAllocInfo, uboDescriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate UBO descriptor sets!");
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferDescInfo = {};
        bufferDescInfo.buffer = uboBuffers[i];
        bufferDescInfo.offset = 0;
        bufferDescInfo.range = sizeof(LightingUBO);

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = uboDescriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferDescInfo;

        vkUpdateDescriptorSets(context->getDevice(), 1, &descriptorWrite, 0, nullptr);
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

void VulkanRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const std::vector<GameObject>& objects, const std::vector<VisualObject>& visualObjects,  const std::vector<Projectile>& projectiles, const glm::mat4& viewMatrix) {
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

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline->getPipelineLayout(), 1, 1, &uboDescriptorSets[currentFrame], 0, nullptr);

    glm::mat4 projection = glm::perspective(
        glm::radians(85.0f),
        static_cast<float>(width) / static_cast<float>(height),
        0.1f,
        100.0f);

    projection[1][1] *= -1;

    for (const GameObject& object : objects) {
        if (object.texture == nullptr || object.texture->getDescriptorSet() == VK_NULL_HANDLE) continue;

        glm::mat4 model;
        model = glm::translate(glm::mat4(1.0f), object.transform.position);
        model *= glm::mat4_cast(object.transform.rotation);
        model = glm::scale(model, object.transform.scale);

        glm::mat4 mvp = projection * viewMatrix * model;
        vkCmdPushConstants(commandBuffer, pipeline->getPipelineLayout(),
            VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &mvp);
        vkCmdPushConstants(commandBuffer, pipeline->getPipelineLayout(),
            VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), sizeof(glm::mat4), &model);

        VkDescriptorSet descriptorSets[] = {object.texture->getDescriptorSet()};
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline->getPipelineLayout(), 0, 1, descriptorSets, 0, nullptr);

        VkBuffer buffers[] = {object.mesh->getBuffer()};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, object.indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, object.indexBuffer->getElementCount(), 1, 0, 0, 0);
    }

    for (const VisualObject& object : visualObjects) {
        if (object.texture == nullptr || object.texture->getDescriptorSet() == VK_NULL_HANDLE) continue;

        glm::mat4 model;
        model = glm::translate(glm::mat4(1.0f), object.transform.position);
        model *= glm::mat4_cast(object.transform.rotation);
        model = glm::scale(model, object.transform.scale);

        glm::mat4 mvp = projection * viewMatrix * model;
        vkCmdPushConstants(commandBuffer, pipeline->getPipelineLayout(),
            VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &mvp);
        vkCmdPushConstants(commandBuffer, pipeline->getPipelineLayout(),
            VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), sizeof(glm::mat4), &model);

        VkDescriptorSet descriptorSets[] = {object.texture->getDescriptorSet()};
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline->getPipelineLayout(), 0, 1, descriptorSets, 0, nullptr);

        VkBuffer buffers[] = {object.mesh->getBuffer()};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, object.indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, object.indexBuffer->getElementCount(), 1, 0, 0, 0);
    }

    for (const Projectile& projectile : projectiles) {
        if (projectile.texture == nullptr || projectile.texture->getDescriptorSet() == VK_NULL_HANDLE) continue;

        glm::mat4 model = glm::translate(glm::mat4(1.0f), projectile.transform.position);
        model *= glm::mat4_cast(projectile.transform.rotation);
        model = glm::scale(model, projectile.transform.scale);

        glm::mat4 mvp = projection * viewMatrix * model;
        vkCmdPushConstants(commandBuffer, pipeline->getPipelineLayout(),
            VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &mvp);
        vkCmdPushConstants(commandBuffer, pipeline->getPipelineLayout(),
            VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), sizeof(glm::mat4), &model);

        VkDescriptorSet descriptorSets[] = {projectile.texture->getDescriptorSet()};
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline->getPipelineLayout(), 0, 1, descriptorSets, 0, nullptr);

        VkBuffer buffers[] = {projectile.mesh->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, projectile.indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, projectile.indexBuffer->getElementCount(), 1, 0, 0, 0);
    }

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to end command buffer!");
    }
}