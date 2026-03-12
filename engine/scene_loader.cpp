#include "scene_loader.h"
#include "vulkan_context.h"
#include "vulkan_pipeline.h"
#include "physics_world.h"
#include "asset_cache.h"
#include "Jolt/Physics/Body/MotionType.h"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

SceneLoader::SceneLoader(VulkanContext* contextIn, VulkanPipeline* pipelineIn, PhysicsWorld* physicsWorldIn, AssetCache* assetCacheIn)
    : context(contextIn), pipeline(pipelineIn), physicsWorld(physicsWorldIn), assetCache(assetCacheIn) {}

void SceneLoader::load(const std::string& levelPath, std::vector<GameObject>& outObjects, std::vector<VisualObject>& outVisuals, std::vector<CollisionZone>& outCollisionZones) {
    std::ifstream levelFile(levelPath);
    json data = json::parse(levelFile);

    if (data.contains("visuals")) {
        for (const auto& visualObject : data["visuals"]) {
            std::string meshPath = visualObject["mesh"];
            std::string texturePath = visualObject["texture"];

            auto [visualMesh, visualIndices] = assetCache->getMesh(meshPath);
            VulkanTexture* visualTexture = assetCache->getTexture(texturePath);

            float posX = visualObject["position"][0], posY = visualObject["position"][1], posZ = visualObject["position"][2];
            float rotW = visualObject["rotation"][0], rotX = visualObject["rotation"][1], rotY = visualObject["rotation"][2], rotZ = visualObject["rotation"][3];
            float scaleX = visualObject["scale"][0],  scaleY = visualObject["scale"][1],  scaleZ = visualObject["scale"][2];

            outVisuals.push_back({
                glm::vec3(posX, posY, posZ),
                glm::quat(rotW, rotX, rotY, rotZ),
                glm::vec3(scaleX, scaleY, scaleZ),
                visualMesh, visualIndices, visualTexture,
                meshPath, texturePath});
        }
    }
    if (data.contains("collisionZones")) {
        for (const auto& collisionZone : data["collisionZones"]) {
            float hx = collisionZone["halfExtents"][0], hy = collisionZone["halfExtents"][1], hz = collisionZone["halfExtents"][2];
            float posX = collisionZone["position"][0], posY = collisionZone["position"][1], posZ = collisionZone["position"][2];

            JPH::BodyID ID = physicsWorld->createBody(
                JPH::Vec3(posX, posY, posZ),
                JPH::Vec3(hx, hy, hz),
                JPH::EMotionType::Static,
                Layers::STATIC,
                "box",
                JPH::EAllowedDOFs::All);

            outCollisionZones.push_back({
            glm::vec3(posX, posY, posZ),
            glm::vec3(hx, hy, hz),
            ID});
        }
    }

    if (data.contains("objects")) {
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

            auto [objectMesh, objectIndices] = assetCache->getMesh(meshPath);

            JPH::BodyID ID = physicsWorld->createBody(
                JPH::Vec3(posX, posY, posZ),
                JPH::Vec3(hx, hy, hz),
                motionType,
                layer,
                shapeType,
                dofs);

            VulkanTexture* objectTexture = assetCache->getTexture(texturePath);

            outObjects.push_back({
                glm::vec3(posX, posY, posZ),
                glm::quat(rotW, rotX, rotY, rotZ),
                glm::vec3(scaleX, scaleY, scaleZ),
                glm::vec3(hx, hy, hz),
                ID, objectMesh, objectIndices, objectTexture});
        }
    }
}

void SceneLoader::save(const std::string& levelPath, const std::string& levelName, std::vector<GameObject>& objectsIn, std::vector<VisualObject>& visualsIn, std::vector<CollisionZone>& collisionZonesIn) {

    nlohmann::json data;
    data["visuals"] = nlohmann::json::array();

    for (auto& visual : visualsIn) {
        glm::vec3 position = visual.transform.position;
        glm::quat rotation = visual.transform.rotation;
        glm::vec3 scale = visual.transform.scale;

        nlohmann::json entry;
        entry["mesh"] = visual.meshPath;
        entry["texture"] = visual.texturePath;
        entry["position"] = {position.x, position.y, position.z};
        entry["rotation"] = {rotation.w, rotation.x, rotation.y, rotation.z};
        entry["scale"] = {scale.x, scale.y, scale.z};
        data["visuals"].push_back(entry);
    }

    data["collisionZones"] = nlohmann::json::array();
    for (auto& zone : collisionZonesIn) {
        glm::vec3 position = zone.position;
        glm::vec3 halfExtents = zone.halfExtents;

        nlohmann::json entry;
        entry["position"] = {position.x, position.y, position.z};
        entry["halfExtents"] = {halfExtents.x, halfExtents.y, halfExtents.z};
        data["collisionZones"].push_back(entry);
    }

    std::ofstream file(levelPath + levelName + ".json");
    file << data.dump(4);
}
