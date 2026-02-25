#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera(glm::vec3 startPos) : position(startPos){}

    glm::vec3 position;
    float yaw = -90.0f;
    float pitch = 0.0f;

    glm::mat4 getViewMatrix() {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(front);

        return glm::lookAt(position, position + front, glm::vec3(0.0f, 1.0f, 0.0f));
    }
};