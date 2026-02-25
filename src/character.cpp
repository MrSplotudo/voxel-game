#include "character.h"
#include "../engine/vulkan_context.h"
#include "../engine/vulkan_pipeline.h"
#include "../engine/physics_layers.h"
#include "../engine/physics_world.h"

#include "Jolt/Physics/Body/AllowedDOFs.h"
#include "Jolt/Physics/Body/BodyInterface.h"
#include "Jolt/Physics/Body/MotionType.h"

Character::Character(VulkanContext* contextIn, VulkanPipeline* pipelineIn, PhysicsWorld* physicsWorldIn) : context(contextIn), pipeline(pipelineIn), physicsWorld(physicsWorldIn) {
    auto [mesh, indexBuffer] = loadMesh(
        "../assets/models/barrel.obj",
        context->getDevice(),
        context->getPhysicalDevice());

    object.mesh = mesh;
    object.indexBuffer = indexBuffer;
    object.texture = new VulkanTexture(
        context->getDevice(),
        context->getPhysicalDevice(),
        context->getGraphicsQueue(),
        context->findQueueFamilies(context->getPhysicalDevice()).graphicsFamily);
    object.texture->load(
        "../assets/textures/barrel_texture.png",
        pipeline->getDescriptorSetLayout());


    auto [gunMesh, gunIndexBuffer] = loadMesh(
        "../assets/models/gun1911.obj",
        context->getDevice(),
        context->getPhysicalDevice());
    gun.mesh = gunMesh;
    gun.indexBuffer = gunIndexBuffer;
    gun.texture = new VulkanTexture(
        context->getDevice(),
        context->getPhysicalDevice(),
        context->getGraphicsQueue(),
        context->findQueueFamilies(context->getPhysicalDevice()).graphicsFamily);
    gun.texture->load(
        "../assets/textures/gun1911.png",
        pipeline->getDescriptorSetLayout());

    bodyInterface = physicsWorld->getBodyInterface();
}

Character::~Character() {
    delete object.mesh;
    delete object.indexBuffer;
    delete object.texture;
    delete gun.mesh;
    delete gun.indexBuffer;
    delete gun.texture;
}

void Character::update(float deltaTime, const InputState& input) {
    JPH::Vec3 vel = bodyInterface->GetLinearVelocity(object.bodyID);
    float velX = vel.GetX();
    float velY = vel.GetY();

    // X movement
    if (input.moveX && abs(velX) < maxSpeed) {
        velX += ((input.moveX * acceleration) * deltaTime);
    }
    if (input.moveX == 0.0f) {
        if (abs(velX) < friction * deltaTime) {
            velX = 0.0f;
        } else {
            velX -= ((friction * (velX > 0.0f ? 1.0f : -1.0f)) * deltaTime);
        }
    }

    // Y movement
    bool grounded = isGrounded();
    if (grounded) {
        jumpsRemaining = jumps;
    }
    if (input.jump&& !lastInput.jump && jumpsRemaining > 0) {
        velY = jumpForce;
        jumpsRemaining--;
    }
    if (!grounded) {
        float gravityMultiplier = velY <= 0.0f ? fallMultiplier : riseMultiplier;
        bodyInterface->SetGravityFactor(object.bodyID, gravityMultiplier);
    }

    lastInput = input;
    bodyInterface->SetLinearVelocity(object.bodyID, JPH::Vec3(velX, velY, 0.0f));
}

void Character::updateGun(const glm::vec3& aimDirection) {
    float angle = atan2(aimDirection.y, aimDirection.x);
    gun.transform.rotation = glm::angleAxis(angle, glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec3 orbitPos = glm::vec3(cos(angle) * gunOrbitRadius, sin(angle) * gunOrbitRadius, 0.0f);
    gun.transform.position = object.transform.position + orbitPos + gunOffset;
}

void Character::spawn(JPH::Vec3 pos) {
    object.bodyID = physicsWorld->createBody(
            JPH::Vec3(pos.GetX(), pos.GetY(), pos.GetZ()),
            JPH::Vec3(0.5f, 0.5f, 0.5f),
            JPH::EMotionType::Dynamic,
            Layers::DYNAMIC,
            "capsule",
            JPH::EAllowedDOFs::TranslationX | JPH::EAllowedDOFs::TranslationY);
}

void Character::despawn() {
    physicsWorld->getBodyInterface()->RemoveBody(object.bodyID);
    physicsWorld->getBodyInterface()->DestroyBody(object.bodyID);
}

glm::vec3 Character::getBarrelTip() {
    glm::vec3 localForward = gun.transform.rotation * glm::vec3(1.0f, 0.0f, 0.0f);
    return gun.transform.position + localForward * gunBarrelLength;
}

bool Character::isGrounded() {
    RayResult result = physicsWorld->castRay(physicsWorld->getPosition(object.bodyID), {0.0f, -1.0f, 0.0f}, 0.55f, object.bodyID);

    return result.hit;
}
