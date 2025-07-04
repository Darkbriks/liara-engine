#pragma once

#include "glm/ext/matrix_float3x3.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/trigonometric.hpp"

namespace Liara::Core::Component
{
    struct TransformComponent3d
    {
        glm::vec3 position{};
        glm::vec3 scale{1.0F, 1.0F, 1.0F};
        glm::vec3 rotation{0.0F};

        // Matrix corresponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        [[nodiscard]] auto GetMat4() const -> glm::mat4 {
            const float c3 = glm::cos(rotation.z);
            const float s3 = glm::sin(rotation.z);
            const float c2 = glm::cos(rotation.x);
            const float s2 = glm::sin(rotation.x);
            const float c1 = glm::cos(rotation.y);
            const float s1 = glm::sin(rotation.y);
            return glm::mat4{
                {scale.x * (c1 * c3 + s1 * s2 * s3), scale.x * (c2 * s3), scale.x * (c1 * s2 * s3 - c3 * s1), 0.0F},
                {scale.y * (c3 * s1 * s2 - c1 * s3), scale.y * (c2 * c3), scale.y * (c1 * c3 * s2 + s1 * s3), 0.0F},
                {scale.z * (c2 * s1),                scale.z * (-s2),     scale.z * (c1 * c2),                0.0F},
                {position.x,                         position.y,          position.z,                         1.0F}
            };
        }

        [[nodiscard]] auto GetNormalMatrix() const -> glm::mat3 {
            const float c3 = glm::cos(rotation.z);
            const float s3 = glm::sin(rotation.z);
            const float c2 = glm::cos(rotation.x);
            const float s2 = glm::sin(rotation.x);
            const float c1 = glm::cos(rotation.y);
            const float s1 = glm::sin(rotation.y);
            const glm::vec3 invScale = 1.0F / scale;

            return glm::mat3{
                {invScale.x * (c1 * c3 + s1 * s2 * s3), invScale.x * (c2 * s3), invScale.x * (c1 * s2 * s3 - c3 * s1)},
                {invScale.y * (c3 * s1 * s2 - c1 * s3), invScale.y * (c2 * c3), invScale.y * (c1 * c3 * s2 + s1 * s3)},
                {invScale.z * (c2 * s1),                invScale.z * (-s2),     invScale.z * (c1 * c2)               }
            };
        }
    };
}
