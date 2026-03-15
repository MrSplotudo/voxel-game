#pragma once
#define GLFW_INCLUDE_VULKAN
#include "Jolt/Jolt.h"
#include "Jolt/Physics/Body/BodyID.h"
#include "transform.h"


struct CollisionZone {
    glm::vec3 position;
    glm::vec3 halfExtents;
    JPH::BodyID bodyID;
};