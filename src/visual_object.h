#pragma once
#define GLFW_INCLUDE_VULKAN
#include "../engine/vulkan_buffer.h"
#include "../engine/vulkan_texture.h"
#include "transform.h"
#include <string>

struct VisualObject {
    Transform transform;
    VulkanBuffer* mesh;
    VulkanBuffer* indexBuffer;
    VulkanTexture* texture;
    std::string meshPath;
    std::string texturePath;
};