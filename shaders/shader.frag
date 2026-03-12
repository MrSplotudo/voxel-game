#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D textureSampler;

layout(set = 1, binding = 0) uniform LightingUBO {
    vec3 lightDirection;
    float padding1;
    vec3 lightColor;
    float padding2;
    vec3 cameraPosition;
    float ambientStrength;
    float specularStrength;
    float shininess;
} lighting;

void main() {
    vec3 normal = normalize(fragNormal);

    vec3 ambient = lighting.ambientStrength * lighting.lightColor;

    float diff = max(dot(normal, lighting.lightDirection), 0.0f);
    vec3 diffuse = diff * lighting.lightColor;

    vec3 viewDir = normalize(lighting.cameraPosition - fragWorldPos);
    vec3 halfwayDir = normalize(lighting.lightDirection + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), lighting.shininess);
    vec3 specular = lighting.specularStrength * spec * lighting.lightColor;

    vec3 texColor = texture(textureSampler, fragUV).rgb;
    vec3 result = (ambient + diffuse + specular) * texColor;

    outColor = vec4(result, 1.0);
}