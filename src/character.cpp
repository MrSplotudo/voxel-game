#include "character.h"
#include "projectile_manager.h"
#include "../engine/vulkan_context.h"
#include "../engine/vulkan_pipeline.h"
#include "../engine/physics_layers.h"
#include "../engine/physics_world.h"
#include "../engine/asset_cache.h"
#include "Jolt/Physics/Body/AllowedDOFs.h"
#include "Jolt/Physics/Body/BodyInterface.h"
#include "Jolt/Physics/Body/MotionType.h"

Character::Character(VulkanContext* contextIn, VulkanPipeline* pipelineIn, PhysicsWorld* physicsWorldIn, AssetCache* assetCacheIn, ProjectileManager* projectileManagerIn)
: context(contextIn), pipeline(pipelineIn), physicsWorld(physicsWorldIn), assetCache(assetCacheIn), projectileManager(projectileManagerIn){

    auto [characterMesh, characterIndices] = assetCache->getMesh("assets/models/barrel.obj");
    object.mesh = characterMesh;
    object.indexBuffer = characterIndices;
    object.texture = assetCache->getTexture("assets/textures/barrel_texture.png");

    auto [gunMesh, gunIndexBuffer] = assetCache->getMesh("assets/models/gun1911.obj");
    gun.mesh = gunMesh;
    gun.transform.scale = glm::vec3(2.0f, 2.0f, 2.0f);
    gun.indexBuffer = gunIndexBuffer;
    gun.texture = assetCache->getTexture("assets/textures/gun1911.png");

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
    grounded = isGrounded();
    if (grounded) {
        bodyInterface->SetGravityFactor(object.bodyID, 1.0f);
        float heightError = lastGroundDistance - hoverTargetHeight;
        float springAccel = -hoverStiffness * heightError - hoverDamping * velY;
        if (springAccel > 0.0f) {
            velY += springAccel * deltaTime;
        }
    }

    if (grounded && !input.jump) {
        jumpsRemaining = jumps;
    }

    if (!grounded) {
        float gravityMultiplier = velY <= 0.0f ? fallMultiplier : riseMultiplier;
        if (input.fastFall) {
            bodyInterface->SetGravityFactor(object.bodyID, gravityMultiplier * 2.0f);
        } else {
            bodyInterface->SetGravityFactor(object.bodyID, gravityMultiplier);
        }
    }

    if (input.jump&& !lastInput.jump && jumpsRemaining > 0) {
        velY = jumpForce;
        jumpsRemaining--;
    }

    if (!input.jump && lastInput.jump && velY > 0.0f) {
        velY *= jumpCutMultiplier;
    }

    if (input.shoot && timeSinceLastFire >= fireCooldown) {
        shoot();
        timeSinceLastFire = 0.0f;
    } else {
        timeSinceLastFire += deltaTime;
    }

    lastInput = input;
    bodyInterface->SetLinearVelocity(object.bodyID, JPH::Vec3(velX, velY, 0.0f));
}

void Character::updateGun(const glm::vec3& aimDirectionIn) {
    aimDirection = aimDirectionIn;

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

void Character::shoot() {
    ProjectileProperties props;
    props.speed = bulletSpeed;
    props.lifespan = bulletLifeTime;
    props.gravity = bulletGravity;
    props.bounces = bounces;
    props.bounciness = bulletBounciness;


    glm::vec3 spawnPos = getBarrelTip();
    projectileManager->spawn(spawnPos, aimDirection, props);
}

glm::vec3 Character::getBarrelTip() {
    glm::vec3 localForward = gun.transform.rotation * glm::vec3(0.0f, 0.0f, 0.0f);
    return gun.transform.position + localForward * gunBarrelLength;
}

bool Character::isGrounded() {
    JPH::Vec3 bodyPos = bodyInterface->GetPosition(object.bodyID);
    float capsuleRadius = 0.5f;
    JPH::Vec3 rayOffsets[3] = {
        JPH::Vec3(0.0f, 0.0f, 0.0f),
        JPH::Vec3(-capsuleRadius, 0.0f, 0.0f),
        JPH::Vec3(capsuleRadius, 0.0f, 0.0f)
    };

    lastGroundDistance = hoverRayLength + 1.0f;
    bool anyHit = false;

    for (int i = 0; i < 3; i++) {
        RayResult hit = physicsWorld->castRay(
            bodyPos + rayOffsets[i], JPH::Vec3(0.0f, -1.0f, 0.0f),
            hoverRayLength, object.bodyID);

        if (hit.hit) {
            anyHit = true;
            if (hit.distance < lastGroundDistance) {
                lastGroundDistance = hit.distance;
            }
        }
    }

    grounded = anyHit && lastGroundDistance < hoverTargetHeight + 0.2f;
    return grounded;
}
