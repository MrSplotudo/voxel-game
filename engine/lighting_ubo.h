#pragma once
#include <glm/glm.hpp>

struct LightingUBO {
    glm::vec3 lightDirection;
    float padding1;

    glm::vec3 lightColor;
    float padding2;

    glm::vec3 cameraPosition;
    float ambientStrength;
    float specularStrength;
    float shininess;
    float padding3;
    float padding4;
};