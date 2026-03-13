#pragma once
#include "projectile.h"
#include "game_object.h"
#include "input_state.h"


class VulkanContext;
class VulkanPipeline;
class PhysicsWorld;
class ProjectileManager;
class AssetCache;
namespace JPH { class BodyInterface;}

class Character {
public:
    Character(VulkanContext* contextIn, VulkanPipeline* pipelineIn, PhysicsWorld* physicsWorldIn, AssetCache* assetCacheIn, ProjectileManager* projectileManagerIn);
    ~Character();

    void update(float deltaTime, const InputState& input);
    void updateGun(const glm::vec3& aimDirectionIn);

    void spawn(JPH::Vec3 pos);
    void despawn();

    void shoot();


    glm::vec3 getBarrelTip();
    bool isGrounded();

    GameObject object;
    GameObject gun;
    glm::vec3 gunOffset = glm::vec3(0.0f, 0.0f, 0.0f);
    float gunBarrelLength = 1.0f;
    float gunOrbitRadius = 1.2f;

    float health = 100.0f;

    float maxSpeed = 10.0f;
    float acceleration = 100.0f;
    float friction = 100.0f;

    float jumpForce = 15.0f;
    int jumps = 1;
    int jumpsRemaining = 0;

    float fallMultiplier = 6.0f;
    float riseMultiplier = 4.0f;
    float jumpCutMultiplier = 0.4f;

    float hoverTargetHeight = 0.8f;
    float hoverStiffness = 100.0f;
    float hoverDamping = 15.0f;
    float hoverRayLength = 1.5f;

    bool grounded = false;
    float lastGroundDistance = 0.0f;

    glm::vec3 aimDirection{};
    bool autoFire = false;
    float timeSinceLastFire = 0.0f;

    InputState lastInput;
    ProjectileProperties projectile;

    float bulletSpeed = 30.0f;
    float bulletLifeTime = 10.0f;
    float bulletGravity = 10.0f;
    int bounces = 2;
    float bulletBounciness = 0.8f;

private:
    VulkanContext* context;
    VulkanPipeline* pipeline;
    PhysicsWorld* physicsWorld;
    ProjectileManager* projectileManager;
    AssetCache* assetCache;
    InputState* input{};
    JPH::BodyInterface* bodyInterface;
};
