#pragma once
#include <memory>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>

#include "Graphics/Liara_Model.h"
#include "Components/TransformComponent3d.h"
#include "Components/RigidBody2dComponent.h"
#include "Components/PointLightComponent.h"

// TODO: Check Entity Component System
namespace Liara::Core
{
    class Liara_GameObject
    {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, Liara_GameObject>;

        static Liara_GameObject CreateGameObject()
        {
            static id_t id = 0;
            return Liara_GameObject(id++);
        }

        static Liara_GameObject MakePointLight(const float intensity = 10.0f, const float radius = 0.01f, const glm::vec3& color = glm::vec3(1.f))
        {
            Liara_GameObject pointLight = CreateGameObject();
            pointLight.m_color = color;
            pointLight.m_Transform.scale.x = radius;
            pointLight.m_PointLight = std::make_unique<Component::PointLightComponent>();
            pointLight.m_PointLight->intensity = intensity;
            return pointLight;
        }

        Liara_GameObject(const Liara_GameObject&) = delete;
        Liara_GameObject& operator=(const Liara_GameObject&) = delete;
        Liara_GameObject(Liara_GameObject&&) = default;
        Liara_GameObject& operator=(Liara_GameObject&&) = default;

        [[nodiscard]] id_t GetId() const { return m_Id; }

        Component::TransformComponent3d m_Transform{};
        glm::vec3 m_color{};

        //Component::RigidBody2dComponent m_RigidBody{};
        std::unique_ptr<Component::PointLightComponent> m_PointLight{};
        std::shared_ptr<Graphics::Liara_Model> m_Model{};

    private:
        id_t m_Id;

        explicit Liara_GameObject(const id_t id) : m_Id(id) {}
    };
}