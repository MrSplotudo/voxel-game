#pragma once
#include "transform.h"
#include "../engine/vulkan_buffer.h"
#include "../engine/vulkan_texture.h"
#include "Jolt/Jolt.h"
#include "Jolt/Physics/Body/BodyID.h"
#include <glm/glm.hpp>


struct ProjectileProperties {
    float damage = 10.0f;
    float speed = 20.0f;
    float size = 0.2f;
    float lifespan = 10.0f;
    float gravity = 4.0f;
    float bounciness = 1.0f;
    int bounces = 0;
    int bouncesRemaining = 0;

};

struct Projectile {
    Transform transform;
    JPH::BodyID bodyID;
    VulkanBuffer* mesh;
    VulkanBuffer* indexBuffer;
    VulkanTexture* texture;
    ProjectileProperties properties;
    float age = 0.0f;
    bool markedForDeletion = false;
};