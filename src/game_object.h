#pragma once
#define GLFW_INCLUDE_VULKAN
#include "../engine/vulkan_buffer.h"
#include "../engine/vulkan_texture.h"
#include "Jolt/Jolt.h"
#include "Jolt/Physics/Body/BodyID.h"
#include "transform.h"

struct GameObject {
    Transform transform;
    glm::vec3 halfExtents;
    JPH::BodyID bodyID;
    VulkanBuffer* mesh;
    VulkanBuffer* indexBuffer;
    VulkanTexture* texture;
};
