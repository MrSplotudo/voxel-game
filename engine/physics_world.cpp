#include "physics_world.h"
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseQuery.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyID.h>

void PhysicsWorld::create() {
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    tempAllocator = new JPH::TempAllocatorImpl(10* 1024 * 1024);
    jobSystemPool = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);
    physicsSystem = new JPH::PhysicsSystem();

    physicsSystem->Init(maxBodies, numBodyMutexes, maxBodyPairs, maxContactConstraints, broadPhaseLayerInterface, objectVsBroadPhaseLayerFilter, objectLayerPairFilter);

    physicsSystem->SetGravity(JPH::Vec3(0.0f, -10.0f, 0.0f));

    bodyInterface = &physicsSystem->GetBodyInterface();
}

void PhysicsWorld::update(float deltaTime) {
    physicsSystem->Update(
        deltaTime,
        1,
        tempAllocator,
        jobSystemPool);
}

void PhysicsWorld::shutdown() {
    delete physicsSystem;
    delete jobSystemPool;
    delete tempAllocator;
}

JPH::BodyID PhysicsWorld::createBody(
    JPH::Vec3 position,
    JPH::Vec3 halfExtents,
    JPH::EMotionType motionType,
    JPH::ObjectLayer layer,
    const std::string& shape,
    JPH::EAllowedDOFs dofs)
{
    JPH::ShapeRefC finalShape;

    if (shape == "capsule") {
        // halfExtents.GetY() = total half-height, halfExtents.GetX() = radius
        float radius = halfExtents.GetX();
        float halfHeight = halfExtents.GetY() - radius; // cylindrical part only
        if (halfHeight < 0.01f) halfHeight = 0.01f;
        JPH::CapsuleShapeSettings shapeSettings(halfHeight, radius);
        finalShape = shapeSettings.Create().Get();
    } else {
        JPH::BoxShapeSettings shapeSettings(halfExtents);
        finalShape = shapeSettings.Create().Get();
    }

    JPH::BodyCreationSettings bodySettings(
        finalShape,
        position,
        JPH::Quat::sIdentity(),
        motionType,
        layer);

    bodySettings.mAllowedDOFs = dofs;

    return bodyInterface->CreateAndAddBody(bodySettings, JPH::EActivation::Activate);
}

JPH::Vec3 PhysicsWorld::getPosition(JPH::BodyID bodyID) {
    return bodyInterface->GetPosition(bodyID);
}

JPH::Quat PhysicsWorld::getRotation(JPH::BodyID bodyID) {
    return bodyInterface->GetRotation(bodyID);
}

RayResult PhysicsWorld::castRay(JPH::Vec3 origin, JPH::Vec3 direction, float maxDistance, JPH::BodyID ignoreBodyID) {
    const JPH::NarrowPhaseQuery& query = physicsSystem->GetNarrowPhaseQuery();

    JPH::RRayCast ray(origin, direction * maxDistance);
    JPH::RayCastResult hitResult;

    JPH::IgnoreSingleBodyFilter bodyFilter(ignoreBodyID);
    bool hit = query.CastRay(ray, hitResult, {}, {}, bodyFilter);


    RayResult result;
    result.hit = hit;
    result.distance = hit ? hitResult.mFraction * maxDistance : 0.0f;
    return result;
}