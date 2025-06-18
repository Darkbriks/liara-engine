#pragma once

#include "Graphics/Liara_Model.h"

#include <memory>
#include <unordered_map>

#include "Components/PointLightComponent.h"
#include "Components/TransformComponent3d.h"
#include "glm/ext/vector_float3.hpp"

// TODO: Check Entity Component System
namespace Liara::Core
{
    class Liara_GameObject
    {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, Liara_GameObject>;

        static Liara_GameObject CreateGameObject() {
            static id_t id = 0;
            return Liara_GameObject(id++);
        }

        static Liara_GameObject MakePointLight(const float intensity = 10.0f,
                                              const float radius = 0.01f,
                                              const glm::vec3& color = glm::vec3(1.f)) {
            Liara_GameObject pointLight = CreateGameObject();
            pointLight.color = color;
            pointLight.transform.scale.x = radius;
            pointLight.pointLight = std::make_unique<Component::PointLightComponent>();
            pointLight.pointLight->intensity = intensity;
            return pointLight;
        }

        ~Liara_GameObject() = default;
        Liara_GameObject(const Liara_GameObject&) = delete;
        Liara_GameObject& operator=(const Liara_GameObject&) = delete;
        Liara_GameObject(Liara_GameObject&&) = default;
        Liara_GameObject& operator=(Liara_GameObject&&) = default;

        [[nodiscard]] id_t GetId() const { return m_Id; }

        Component::TransformComponent3d transform{};
        glm::vec3 color{};

        std::unique_ptr<Component::PointLightComponent> pointLight;
        std::shared_ptr<Graphics::Liara_Model> model;

    private:
        id_t m_Id;

        explicit Liara_GameObject(const id_t id)
            : m_Id(id) {}
    };
}