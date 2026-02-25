#pragma once
#include "projectile.h"
#include "game_object.h"
#include "input_state.h"


class VulkanContext;
class VulkanPipeline;
class PhysicsWorld;
namespace JPH { class BodyInterface;}

class Character {
public:
    Character(VulkanContext* contextIn, VulkanPipeline* pipelineIn, PhysicsWorld* physicsWorldIn);
    ~Character();

    void update(float deltaTime, const InputState& input);
    void updateGun(const glm::vec3& aimDirection);

    void spawn(JPH::Vec3 pos);
    void despawn();

    glm::vec3 getBarrelTip();

    bool isGrounded();

    GameObject object;
    GameObject gun;
    glm::vec3 gunOffset = glm::vec3(0.0f, 0.0f, 0.0f);
    float gunBarrelLength = 1.0f;
    float gunOrbitRadius = 1.2f;


    float maxSpeed = 10;
    float acceleration = 100;
    float friction = 100;

    float jumpForce = 15;
    int jumps = 1;
    int jumpsRemaining = 0;

    float fallMultiplier = 6.0f;
    float riseMultiplier = 4.0f;

    float health;



    InputState lastInput;
    ProjectileProperties projectile;

private:
    VulkanContext* context;
    VulkanPipeline* pipeline;
    PhysicsWorld* physicsWorld;
    JPH::BodyInterface* bodyInterface;
    InputState* input;
};
