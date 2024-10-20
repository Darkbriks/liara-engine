//
// Created by antoi on 20/10/2024.
//

#include "GravityVecFieldApp.h"
#include "../../SimpleRenderSystem.h"

#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "GravityPhysicsSystem.h"

namespace Liara
{
    void GravityVecFieldApp::Run()
    {
        // create some models
        std::shared_ptr<Liara_Model> squareModel = CreateSquareModel(m_Device, {0.5f, 0.0f});  // offset model by .5 so rotation occurs at edge rather than center of square
        std::shared_ptr<Liara_Model> circleModel = CreateCircleModel(m_Device, 64);

        // create physics objects
        std::vector<Liara_GameObject> physicsObjects{};
        auto red = Liara_GameObject::CreateGameObject();
        red.m_Transform.scale = glm::vec2{.05f};
        red.m_Transform.position = {.5f, .5f};
        red.m_color = {1.f, 0.f, 0.f};
        red.m_RigidBody.velocity = {-.4f, .0f};
        red.m_Model = circleModel;
        physicsObjects.push_back(std::move(red));
        auto blue = Liara_GameObject::CreateGameObject();
        blue.m_Transform.scale = glm::vec2{.05f};
        blue.m_Transform.position = {-.45f, -.25f};
        blue.m_color = {0.f, 0.f, 1.f};
        blue.m_RigidBody.velocity = {.4f, .0f};
        blue.m_Model = circleModel;
        physicsObjects.push_back(std::move(blue));

        // create vector field
        std::vector<Liara_GameObject> vectorField{};
        int gridCount = 40;
        for (int i = 0; i < gridCount; i++)
        {
            for (int j = 0; j < gridCount; j++)
            {
                auto vf = Liara_GameObject::CreateGameObject();
                vf.m_Transform.scale = glm::vec2(0.005f);
                vf.m_Transform.position = {
                    -1.0f + (static_cast<float>(i) + 0.5f) * 2.0f / static_cast<float>(gridCount),
                    -1.0f + (static_cast<float>(j) + 0.5f) * 2.0f / static_cast<float>(gridCount)};
                vf.m_color = glm::vec3(1.0f);
                vf.m_Model = squareModel;
                vectorField.push_back(std::move(vf));
            }
        }

        GravityPhysicsSystem gravitySystem{0.81f};
        Vec2FieldSystem vecFieldSystem{};

        const SimpleRenderSystem render_system{m_Device, m_Renderer.GetSwapChainRenderPass()};

        while (!m_Window.ShouldClose())
        {
            glfwPollEvents();

            if (const auto commandBuffer = m_Renderer.BeginFrame())
            {
                // update systems
                gravitySystem.Update(physicsObjects, 1.f / 60, 5);
                vecFieldSystem.Update(gravitySystem, physicsObjects, vectorField);

                // render system
                m_Renderer.BeginSwapChainRenderPass(commandBuffer);
                render_system.RenderGameObjects(commandBuffer, physicsObjects);
                render_system.RenderGameObjects(commandBuffer, vectorField);
                m_Renderer.EndSwapChainRenderPass(commandBuffer);
                m_Renderer.EndFrame();
            }
        }

        vkDeviceWaitIdle(m_Device.GetDevice());
    }
}