#include "projectile_manager.h"
#include "../engine/physics_world.h"
#include "../engine/physics_layers.h"
#include "../engine/vulkan_context.h"
#include "../engine/vulkan_pipeline.h"
#include "../engine/vulkan_buffer.h"
#include "../engine/vulkan_texture.h"
#include "Jolt/Physics/Body/MotionType.h"
#include "Jolt/Physics/Body/AllowedDOFs.h"
#include "Jolt/Physics/Body/BodyInterface.h"


ProjectileManager::ProjectileManager(PhysicsWorld* physicsWorldIn, VulkanContext* contextIn, VulkanPipeline* pipelineIn) : physicsWorld(physicsWorldIn), context(contextIn), pipeline(pipelineIn) {

}

void ProjectileManager::init(const std::string& meshPath, const std::string& texturePath) {
    auto [mesh, indexBuffer] = loadMesh(meshPath, context->getDevice(), context->getPhysicalDevice());
    sharedMesh = mesh;
    sharedIndexBuffer = indexBuffer;

    sharedTexture = new VulkanTexture(
        context->getDevice(),
        context->getPhysicalDevice(),
        context->getGraphicsQueue(),
        context->findQueueFamilies(context->getPhysicalDevice()).graphicsFamily);

    sharedTexture->load(texturePath, pipeline->getDescriptorSetLayout());
}

void ProjectileManager::shutdown() {
    for (auto& projectile : projectiles) {
        destroyProjectile(projectile);
    }
    projectiles.clear();

    delete sharedMesh;
    delete sharedIndexBuffer;
    delete sharedTexture;
}

void ProjectileManager::spawn(glm::vec3 position, glm::vec3 direction, ProjectileProperties properties) {
    float radius = properties.size;

    JPH::BodyID bodyID = physicsWorld->createBody(
        JPH::Vec3(position.x, position.y, position.z),
        JPH::Vec3(radius, radius, radius),
        JPH::EMotionType::Dynamic,
        Layers::DYNAMIC,
        "box",
        JPH::EAllowedDOFs::TranslationX | JPH::EAllowedDOFs::TranslationY);

    glm::vec3 velocity = glm::normalize(direction) * properties.speed;
    physicsWorld->getBodyInterface()->SetLinearVelocity(
        bodyID,
        JPH::Vec3(velocity.x, velocity.y, velocity.z));

    physicsWorld->getBodyInterface()->SetGravityFactor(bodyID, 0.0f);

    Projectile projectile;
    projectile.bodyID = bodyID;
    projectile.mesh = sharedMesh;
    projectile.indexBuffer = sharedIndexBuffer;
    projectile.texture = sharedTexture;
    projectile.properties = properties;
    projectile.age = 0.0f;
    projectile.transform.position = position;

    projectiles.push_back(projectile);
}

void ProjectileManager::update(float deltaTime) {
    for (auto& projectile : projectiles) {
        projectile.age += deltaTime;

        if (projectile.age >= projectile.properties.lifespan) {
            projectile.markedForDeletion = true;
        }

        if (!projectile.markedForDeletion) {
            JPH::Vec3 physPos = physicsWorld->getPosition(projectile.bodyID);
            JPH::Quat physRot = physicsWorld->getRotation(projectile.bodyID);
            projectile.transform.position = glm::vec3(physPos.GetX(), physPos.GetY(), physPos.GetZ());
            projectile.transform.rotation = glm::quat(physRot.GetW(), physRot.GetX(), physRot.GetY(), physRot.GetZ());
        }
    }

    // Clean up bodies before erasing
    for (auto& projectile : projectiles) {
        if (projectile.markedForDeletion) {
            destroyProjectile(projectile);
        }
    }

    std::erase_if(projectiles, [](const Projectile& projectile) {
        return projectile.markedForDeletion;
    });
}

void ProjectileManager::destroyProjectile(Projectile& projectile) {
    physicsWorld->getBodyInterface()->RemoveBody(projectile.bodyID);
    physicsWorld->getBodyInterface()->DestroyBody(projectile.bodyID);
}
