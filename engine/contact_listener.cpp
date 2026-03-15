#include "contact_listener.h"
#include "Jolt/Physics/Body/Body.h"
#include <iostream>


void MyContactListener::OnContactAdded(const JPH::Body& body1, const JPH::Body& body2,
                                       const JPH::ContactManifold& manifold, JPH::ContactSettings& settings) {
    if (onContact) {
        onContact(body1, body2);
    }
}
