#pragma once

#define MAX_LIGHTS 10

namespace Liara::Graphics::Ubo
{
    struct PointLight
    {
        glm::vec4 position{}; // ignore w
        glm::vec4 color{}; // w is intensity
    };

    struct GlobalUbo
    {
        glm::mat4 projection{1.f};
        glm::mat4 view{1.f};
        glm::vec4 directionalLightDirection = glm::vec4(glm::normalize(glm::vec3(1.0f, -3.0f, -1.0f)), 0.2f); // xyz is direction, w is intensity
        glm::vec4 directionalLightColor{1.f, 1.f, 1.f, .01f}; // w is ambient intensity
        PointLight pointLights[MAX_LIGHTS];
        int numLights;
    };
}