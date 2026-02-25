#include "processInput.h"
#include "glm/ext/matrix_projection.hpp"

ProcessInput::ProcessInput(GLFWwindow* windowIn ,JPH::BodyInterface* bodyInterfaceIn) : window(windowIn), bodyInterface(bodyInterfaceIn) {

}

glm::vec3 ProcessInput::getWorldCursorPos(const glm::mat4& view, const glm::mat4& projection, uint32_t screenWidth, uint32_t screenHeight) {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    mouseY = screenHeight - mouseY;

    glm::vec4 viewport = glm::vec4(0.0f, 0.0f, screenWidth, screenHeight);


    glm::vec3 nearPoint = glm::unProject(glm::vec3(mouseX, mouseY, 0.0f), view, projection, viewport);
    glm::vec3 farPoint = glm::unProject(glm::vec3(mouseX, mouseY, 1.0f), view, projection, viewport);

    glm::vec3 rayDir = glm::normalize(farPoint - nearPoint);

    float t = -nearPoint.z / rayDir.z;
    glm::vec3 worldPos = nearPoint + t * rayDir;
    return worldPos;
}

InputState ProcessInput::getInputState() {
    InputState input{};
    // X axis
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) input.moveX += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) input.moveX -= 1.0f;

    // Y axis
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) input.jump = true;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) input.fastFall = true;

    return input;
}
