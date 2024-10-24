//
// Created by antoi on 19/10/2024.
//

#pragma once
#include <memory>
#include <glm/gtc/matrix_transform.hpp>

#include "Graphics/Liara_Model.h"

// TODO: Check Entity Component System
namespace Liara::Core
{
    struct TransformComponent3d
    {
        glm::vec3 position{};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::vec3 rotation{0.0f};

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        [[nodiscard]] glm::mat4 GetMat4() const
        {
            const float c3 = glm::cos(rotation.z);
            const float s3 = glm::sin(rotation.z);
            const float c2 = glm::cos(rotation.x);
            const float s2 = glm::sin(rotation.x);
            const float c1 = glm::cos(rotation.y);
            const float s1 = glm::sin(rotation.y);
            return glm::mat4{
            { scale.x * (c1 * c3 + s1 * s2 * s3), scale.x * (c2 * s3), scale.x * (c1 * s2 * s3 - c3 * s1), 0.0f},
            { scale.y * (c3 * s1 * s2 - c1 * s3), scale.y * (c2 * c3), scale.y * (c1 * c3 * s2 + s1 * s3), 0.0f },
            { scale.z * (c2 * s1), scale.z * (-s2), scale.z * (c1 * c2), 0.0f },
            {position.x, position.y, position.z, 1.0f}};
        }
    };

    struct RigidBody2dComponent
    {
        glm::vec2 velocity;
        float mass{1.0f};
    };

    class Liara_GameObject
    {
    public:
        using id_t = unsigned int;

        static Liara_GameObject CreateGameObject()
        {
            static id_t id = 0;
            return Liara_GameObject(id++);
        }

        Liara_GameObject(const Liara_GameObject&) = delete;
        Liara_GameObject& operator=(const Liara_GameObject&) = delete;
        Liara_GameObject(Liara_GameObject&&) = default;
        Liara_GameObject& operator=(Liara_GameObject&&) = default;

        [[nodiscard]] id_t GetId() const { return m_Id; }

        TransformComponent3d m_Transform{};
        RigidBody2dComponent m_RigidBody{};
        std::shared_ptr<Graphics::Liara_Model> m_Model{};
        glm::vec3 m_color{};

    private:
        id_t m_Id;

        explicit Liara_GameObject(const id_t id) : m_Id(id) {}
    };
}