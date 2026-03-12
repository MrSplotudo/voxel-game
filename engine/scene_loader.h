#pragma once
#include "../src/game_object.h"
#include "../src/visual_object.h"
#include "../src/collision_zone.h"
#include <string>
#include <vector>

class VulkanContext;
class VulkanPipeline;
class PhysicsWorld;
class AssetCache;

class SceneLoader {
public:
    SceneLoader(VulkanContext* contextIn, VulkanPipeline* pipelineIn, PhysicsWorld* physicsWorldIn, AssetCache* assetCacheIn);

    void load(const std::string& levelPath, std::vector<GameObject>& outObjects, std::vector<VisualObject>& outVisuals, std::vector<CollisionZone>& outCollisionZones);
    void save(const std::string& levelPath, const std::string& levelName, std::vector<GameObject>& objectsIn, std::vector<VisualObject>& visualsIn, std::vector<CollisionZone>& collisionZonesIn);

private:
    VulkanContext* context;
    VulkanPipeline* pipeline;
    PhysicsWorld* physicsWorld;
    AssetCache* assetCache;
};

