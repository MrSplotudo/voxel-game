#pragma once
#include "projectile.h"
#include <deque>

class PhysicsWorld;
class VulkanContext;
class VulkanPipeline;

class ProjectileManager {
public:
    ProjectileManager(PhysicsWorld* physicsWorldIn, VulkanContext* contextIn, VulkanPipeline* pipelineIn);

    void init(const std::string& meshPath, const std::string& texturePath);
    void shutdown();

    void spawn(glm::vec3 position, glm::vec3 direction, ProjectileProperties properties);
    void update(float deltaTime);

    const std::deque<std::unique_ptr<Projectile>>& getProjectiles() const { return projectiles; }

private:
    void destroyProjectile(Projectile& projectile);

    PhysicsWorld* physicsWorld;
    VulkanContext* context;
    VulkanPipeline* pipeline;

    std::deque<std::unique_ptr<Projectile>> projectiles;

    VulkanBuffer* sharedMesh = nullptr;
    VulkanBuffer* sharedIndexBuffer = nullptr;
    VulkanTexture* sharedTexture = nullptr;
};