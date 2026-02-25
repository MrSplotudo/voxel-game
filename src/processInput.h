#pragma once

#include "input_state.h"
#include <GLFW/glfw3.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <glm/glm.hpp>


class Camera;

class ProcessInput {
public:
    ProcessInput(GLFWwindow* windowIn, JPH::BodyInterface* bodyInterfaceIn);
    glm::vec3 getWorldCursorPos(const glm::mat4& view, const glm::mat4& projection, uint32_t screenWidth, uint32_t screenHeight);
    InputState getInputState();
    float aimAngle = 0.0f;

private:
    GLFWwindow* window;
    JPH::BodyInterface* bodyInterface = nullptr;
};