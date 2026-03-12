#include "debug_ui.h"
#include <glm/glm.hpp>
#include "imgui.h"
#include "glm/ext/matrix_clip_space.hpp"
#include <stdexcept>

DebugUI::DebugUI(GLFWwindow* windowIn, VkInstance instanceIn, VkPhysicalDevice physicalDeviceIn, VkDevice deviceIn, VkQueue queueIn, VkRenderPass renderPassIn, uint32_t imageCountIn, uint32_t queueFamilyIndexIn) : window(windowIn), instance(instanceIn), physicalDevice(physicalDeviceIn), device(deviceIn), graphicsQueue(queueIn), renderPass(renderPassIn), imageCount(imageCountIn), queueFamilyIndex(queueFamilyIndexIn) {

}

DebugUI::~DebugUI() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}

void DebugUI::create() {
    ImGui_ImplVulkan_InitInfo imGuiInitInfo = {};
    imGuiInitInfo.Instance = instance;
    imGuiInitInfo.PhysicalDevice = physicalDevice;
    imGuiInitInfo.Device = device;
    imGuiInitInfo.QueueFamily = queueFamilyIndex;
    imGuiInitInfo.Queue = graphicsQueue;
    imGuiInitInfo.PipelineInfoMain.RenderPass = renderPass;
    imGuiInitInfo.ImageCount = imageCount;
    imGuiInitInfo.MinImageCount = imageCount;

    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 100;
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 100;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
    imGuiInitInfo.DescriptorPool = descriptorPool;

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_Init(&imGuiInitInfo);
}

void DebugUI::beginFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void DebugUI::endFrame() {
    ImGui::Render();
}

void DebugUI::renderHitboxes(glm::mat4 vp, float width, float height, std::vector<GameObject>& gameObjects, std::vector<CollisionZone>& collisionZones) {
    // Build the same projection matrix your renderer uses
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    // The 12 edges of a box, as pairs of corner indices (0-7)
    // Bottom face: 0-1, 1-3, 3-2, 2-0
    // Top face:    4-5, 5-7, 7-6, 6-4
    // Verticals:   0-4, 1-5, 2-6, 3-7
    int edges[12][2] = {
        {0,1}, {1,3}, {3,2}, {2,0},  // bottom
        {4,5}, {5,7}, {7,6}, {6,4},  // top
        {0,4}, {1,5}, {2,6}, {3,7}   // verticals
    };

    for (const auto& obj : gameObjects) {
        glm::vec3 pos = obj.transform.position;
        glm::vec3 he = obj.halfExtents;

        // Build 8 corners: every combination of ±halfExtents
        glm::vec3 corners[8] = {
            pos + glm::vec3(-he.x, -he.y, -he.z),  // 0: bottom-left-back
            pos + glm::vec3(+he.x, -he.y, -he.z),  // 1: bottom-right-back
            pos + glm::vec3(-he.x, -he.y, +he.z),  // 2: bottom-left-front
            pos + glm::vec3(+he.x, -he.y, +he.z),  // 3: bottom-right-front
            pos + glm::vec3(-he.x, +he.y, -he.z),  // 4: top-left-back
            pos + glm::vec3(+he.x, +he.y, -he.z),  // 5: top-right-back
            pos + glm::vec3(-he.x, +he.y, +he.z),  // 6: top-left-front
            pos + glm::vec3(+he.x, +he.y, +he.z),  // 7: top-right-front
        };

        // Project all 8 corners to screen space
        ImVec2 screenPoints[8];
        bool valid[8];

        for (int i = 0; i < 8; i++) {
            glm::vec4 clip = vp * glm::vec4(corners[i], 1.0f);

            // Behind camera check
            if (clip.w <= 0.0f) {
                valid[i] = false;
                continue;
            }

            valid[i] = true;
            float ndcX = clip.x / clip.w;
            float ndcY = clip.y / clip.w;
            screenPoints[i] = ImVec2(
                (ndcX * 0.5f + 0.5f) * width,
                (ndcY * 0.5f + 0.5f) * height
            );
        }

        // Draw 12 edges
        for (auto& edge : edges) {
            int a = edge[0], b = edge[1];
            if (valid[a] && valid[b]) {
                drawList->AddLine(
                    screenPoints[a], screenPoints[b],
                    IM_COL32(255, 0, 0, 255), 2.0f);
            }
        }
    }

    for (const auto& obj : collisionZones) {
        glm::vec3 pos = obj.position;
        glm::vec3 he = obj.halfExtents;

        // Build 8 corners: every combination of ±halfExtents
        glm::vec3 corners[8] = {
            pos + glm::vec3(-he.x, -he.y, -he.z),  // 0: bottom-left-back
            pos + glm::vec3(+he.x, -he.y, -he.z),  // 1: bottom-right-back
            pos + glm::vec3(-he.x, -he.y, +he.z),  // 2: bottom-left-front
            pos + glm::vec3(+he.x, -he.y, +he.z),  // 3: bottom-right-front
            pos + glm::vec3(-he.x, +he.y, -he.z),  // 4: top-left-back
            pos + glm::vec3(+he.x, +he.y, -he.z),  // 5: top-right-back
            pos + glm::vec3(-he.x, +he.y, +he.z),  // 6: top-left-front
            pos + glm::vec3(+he.x, +he.y, +he.z),  // 7: top-right-front
        };

        // Project all 8 corners to screen space
        ImVec2 screenPoints[8];
        bool valid[8];

        for (int i = 0; i < 8; i++) {
            glm::vec4 clip = vp * glm::vec4(corners[i], 1.0f);

            // Behind camera check
            if (clip.w <= 0.0f) {
                valid[i] = false;
                continue;
            }

            valid[i] = true;
            float ndcX = clip.x / clip.w;
            float ndcY = clip.y / clip.w;
            screenPoints[i] = ImVec2(
                (ndcX * 0.5f + 0.5f) * width,
                (ndcY * 0.5f + 0.5f) * height
            );
        }

        // Draw 12 edges
        for (auto& edge : edges) {
            int a = edge[0], b = edge[1];
            if (valid[a] && valid[b]) {
                drawList->AddLine(
                    screenPoints[a], screenPoints[b],
                    IM_COL32(255, 0, 0, 255), 2.0f);
            }
        }

    }
}
