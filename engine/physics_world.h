#pragma once
#include "Jolt/Jolt.h"
#include "physics_layers.h"
#include <string>

#include "contact_listener.h"
#include "Jolt/Physics/Body/AllowedDOFs.h"
#include "Jolt/Physics/Body/BodyID.h"

namespace JPH {
enum class EAllowedDOFs : uint8;
enum class EMotionType : uint8;
class BodyInterface;
class BodyID;
class TempAllocatorImpl;
class JobSystemThreadPool;
class PhysicsSystem;
}
class MyContactListener;


struct RayResult {
    bool hit = false;
    float distance = 0.0f;
};


class PhysicsWorld {
public:
    void create();
    void update(float deltaTime);
    void shutdown();

    JPH::BodyID createBody(
        JPH::Vec3 position,
        JPH::Vec3 halfExtents,
        JPH::EMotionType motionType,
        JPH::ObjectLayer layer,
        const std::string& shape = "box",
        JPH::EAllowedDOFs dofs = JPH::EAllowedDOFs::All);

    JPH::Vec3 getPosition(JPH::BodyID bodyID);
    JPH::Quat getRotation(JPH::BodyID bodyID);
    JPH::BodyInterface* getBodyInterface() { return bodyInterface; }
    MyContactListener* getContactListener() { return myContactListener; }

    RayResult castRay(JPH::Vec3 origin, JPH::Vec3 direction, float maxDistance, JPH::BodyID ignoreBodyID = JPH::BodyID());


private:
    JPH::TempAllocatorImpl* tempAllocator = nullptr;
    JPH::JobSystemThreadPool* jobSystemPool = nullptr;
    JPH::PhysicsSystem* physicsSystem = nullptr;
    JPH::BodyInterface* bodyInterface = nullptr;

    MyContactListener* myContactListener = nullptr;

    const JPH::uint maxBodies = 1024;
    const JPH::uint numBodyMutexes = 0;
    const JPH::uint maxBodyPairs = 1024;
    const JPH::uint maxContactConstraints = 1024;

    BroadPhaseLayerInterfaceImpl broadPhaseLayerInterface;
    ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhaseLayerFilter;
    ObjectLayerPairFilterImpl objectLayerPairFilter;
};





