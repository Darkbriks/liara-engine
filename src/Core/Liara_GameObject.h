#pragma once
#include <memory>
#include <glm/gtc/matrix_transform.hpp>

#include "Graphics/Liara_Model.h"
#include "Components/TransformComponent3d.h"
#include "Components/RigidBody2dComponent.h"

// TODO: Check Entity Component System
namespace Liara::Core
{
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

        Component::TransformComponent3d m_Transform{};
        Component::RigidBody2dComponent m_RigidBody{};
        std::shared_ptr<Graphics::Liara_Model> m_Model{};
        glm::vec3 m_color{};

    private:
        id_t m_Id;

        explicit Liara_GameObject(const id_t id) : m_Id(id) {}
    };
}