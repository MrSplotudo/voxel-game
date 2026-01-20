#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 position;
    float yaw = -90.0f;
    float pitch = 0.0f;
    float speed = 2.5f;

    Camera(glm::vec3 startPosition) : position(startPosition) {}

    glm::mat4 getViewMatrix() {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(front);

        return glm::lookAt(position, position + front, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void processKeyboard(GLFWwindow* window, float deltaTime) {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw));
        front.y = 0.0f;
        front.z = sin(glm::radians(yaw));
        front = glm::normalize(front);

        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
        float velocity = speed * deltaTime;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            position += front * velocity;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            position -= front * velocity;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            position -= right * velocity;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            position += right * velocity;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            position.y += velocity;
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            position.y -= velocity;
    }

    void processMouse(float xOffset, float yOffset) {
        float sensitivity = 0.1f;
        yaw += xOffset * sensitivity;
        pitch -= yOffset * sensitivity;

        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }
};