#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <functional>

namespace JPH {
class Body;
class ContactManifold;
class ContactSettings;
}

class MyContactListener : public JPH::ContactListener {
public:
    std::function<void(const JPH::Body&, const JPH::Body&)> onContact;

    void OnContactAdded(const JPH::Body &body1, const JPH::Body &body2, const JPH::ContactManifold &manifold, JPH::ContactSettings &settings) override;
};