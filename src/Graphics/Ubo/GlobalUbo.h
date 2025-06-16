/**
 * @file GlobalUbo.h
 * @brief Global uniform buffer object for the graphics pipeline.
 *
 * This file contains the definition of the global uniform buffer object (UBO) structure used in the graphics pipeline.
 *
 * A uniform buffer object is a buffer that contains uniform data that is shared between the CPU and the GPU.
 */

#pragma once

#include "../GraphicsConstants.h"
#include <glm/glm.hpp>

namespace Liara::Graphics::Ubo
{
    /**
     * @struct PointLight
     * @brief Structure representing a point light in the scene.
     */
    struct PointLight
    {
        glm::vec4 position{};       ///< The position of the light, w is ignored
        glm::vec4 color{};          ///< The color of the light, w represents the intensity of the light
    };

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4324) // Structure was padded due to alignment specifier
#endif
    /**
     * @struct GlobalUbo
     * @brief Structure representing the global uniform buffer object for the graphics pipeline.
     */
    struct alignas(Constants::UNIFORM_BUFFER_ALIGNMENT) GlobalUbo
    {
        glm::mat4 projection{1.f};                                     ///< The projection matrix
        glm::mat4 view{1.f};                                           ///< The view matrix
        glm::mat4 inverseView{1.f};                                    ///< The inverse view matrix
        glm::vec4 directionalLightDirection = glm::vec4(glm::normalize(glm::vec3(1.0f, -3.0f, -1.0f)), 0.0f); ///< The direction of the directional light, w is ignored
        glm::vec4 directionalLightColor{1.f, 1.f, 1.f, .01f};   ///< The color of the directional light, w represents the intensity of the light

        PointLight pointLights[Constants::MAX_LIGHTS];           ///< The point lights in the scene
        int numLights{0};                                                ///< The number of point lights in the scene

        int32_t _padding[3]{0, 0, 0};
    };

#if defined(_MSC_VER)
#pragma warning(pop) // Restore previous warning state
#endif

    static_assert(sizeof(GlobalUbo) % Constants::UNIFORM_BUFFER_ALIGNMENT == 0,
                     "GlobalUbo size must be aligned to uniform buffer requirements");
}

