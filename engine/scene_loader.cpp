#include "scene_loader.h"
#include "vulkan_context.h"
#include "vulkan_pipeline.h"
#include "physics_world.h"
#include "Jolt/Physics/Body/MotionType.h"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

SceneLoader::SceneLoader(VulkanContext* contextIn, VulkanPipeline* pipelineIn, PhysicsWorld* physicsWorldIn)
    : context(contextIn), pipeline(pipelineIn), physicsWorld(physicsWorldIn) {}

void SceneLoader::load(const std::string& levelPath, std::vector<GameObject>& outObjects) {
    std::ifstream levelFile(levelPath);
    json data = json::parse(levelFile);

    for (const auto& object : data["objects"]) {
        std::string meshPath   = object["mesh"];
        std::string texturePath = object["texture"];

        float posX = object["position"][0], posY = object["position"][1], posZ = object["position"][2];
        float rotW = object["rotation"][0], rotX = object["rotation"][1], rotY = object["rotation"][2], rotZ = object["rotation"][3];
        float scaleX = object["scale"][0],  scaleY = object["scale"][1],  scaleZ = object["scale"][2];

        std::string dataMotionType = object["physics"]["motionType"];
        JPH::EMotionType motionType;
        JPH::ObjectLayer layer;
        if (dataMotionType == "static") {
            motionType = JPH::EMotionType::Static;
            layer = Layers::STATIC;
        } else {
            motionType = JPH::EMotionType::Dynamic;
            layer = Layers::DYNAMIC;
        }

        float hx = object["physics"]["halfExtents"][0];
        float hy = object["physics"]["halfExtents"][1];
        float hz = object["physics"]["halfExtents"][2];

        std::string shapeType = object["physics"].value("shape", "box");

        // 2D constraint: dynamic objects only move on X/Y and rotate on Z
        JPH::EAllowedDOFs dofs = JPH::EAllowedDOFs::All;
        if (motionType == JPH::EMotionType::Dynamic) {
            dofs = JPH::EAllowedDOFs::TranslationX
                 | JPH::EAllowedDOFs::TranslationY;
        }

        auto [objectMesh, objectIndices] = loadMesh("../" + meshPath, context->getDevice(), context->getPhysicalDevice());

        JPH::BodyID ID = physicsWorld->createBody(
            JPH::Vec3(posX, posY, posZ),
            JPH::Vec3(hx, hy, hz),
            motionType,
            layer,
            shapeType,
            dofs);

        VulkanTexture* objectTexture = new VulkanTexture(
            context->getDevice(),
            context->getPhysicalDevice(),
            context->getGraphicsQueue(),
            context->findQueueFamilies(context->getPhysicalDevice()).graphicsFamily);

        objectTexture->load("../" + texturePath, pipeline->getDescriptorSetLayout());

        outObjects.push_back({
            glm::vec3(posX, posY, posZ),
            glm::quat(rotW, rotX, rotY, rotZ),
            glm::vec3(scaleX, scaleY, scaleZ),
            ID, objectMesh, objectIndices, objectTexture});
    }
}