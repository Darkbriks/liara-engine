//
// Created by antoi on 19/10/2024.
//

#pragma once
#include <memory>

#include "Liara_Model.h"

// TODO: Check Entity Component System
namespace Liara
{
    struct Transform2D
    {
        glm::vec2 position{};
        glm::vec2 scale{1.0f, 1.0f};
        float rotation{0.0f};

        [[nodiscard]] glm::mat2 GetMat2() const
        {
            const float cos = std::cos(rotation);
            const float sin = std::sin(rotation);
            const glm::mat2 rotationMat = {{cos, sin}, {-sin, cos}};
            const glm::mat2 scaleMat = {{scale.x, 0.0f}, {0.0f, scale.y}};
            return rotationMat * scaleMat;
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

        Transform2D m_Transform{};
        RigidBody2dComponent m_RigidBody{};
        std::shared_ptr<Liara_Model> m_Model{};
        glm::vec3 m_color{};

    private:
        id_t m_Id;

        explicit Liara_GameObject(const id_t id) : m_Id(id) {}
    };
}