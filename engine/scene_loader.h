#pragma once
#include "../src/game_object.h"
#include <string>
#include <vector>

class VulkanContext;
class VulkanPipeline;
class PhysicsWorld;

class SceneLoader {
public:
    SceneLoader(VulkanContext* contextIn, VulkanPipeline* pipelineIn, PhysicsWorld* physicsWorldIn);

    void load(const std::string& levelPath, std::vector<GameObject>& outObjects);

private:
    VulkanContext* context;
    VulkanPipeline* pipeline;
    PhysicsWorld* physicsWorld;
};

