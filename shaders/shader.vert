#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragWorldPos;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    mat4 model;
} pushConstants;

void main() {
    gl_Position = pushConstants.mvp * vec4(inPosition, 1.0f);
    fragUV = inUV;
    fragNormal = mat3(pushConstants.model) * inNormal;
    fragWorldPos = vec3(pushConstants.model * vec4(inPosition, 1.0f));
}