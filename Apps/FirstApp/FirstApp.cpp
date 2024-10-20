//
// Created by antoi on 15/10/2024.
//

#include "FirstApp.h"
#include "../../SimpleRenderSystem.h"

#include <array>
#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Liara
{
    FirstApp::FirstApp()
    {
        LoadGameObjects();
    }

    void FirstApp::Run()
    {
        const SimpleRenderSystem render_system{m_Device, m_Renderer.GetSwapChainRenderPass()};

        while (!m_Window.ShouldClose())
        {
            glfwPollEvents();

            int i = 0;
            for (auto& obj : m_GameObjects)
            {
                i++;
                obj.m_Transform.rotation = glm::mod<float>(obj.m_Transform.rotation + 0.0005f * static_cast<float>(i), 2.f * glm::pi<float>());
            }

            if (const auto commandBuffer = m_Renderer.BeginFrame())
            {
                m_Renderer.BeginSwapChainRenderPass(commandBuffer);
                render_system.RenderGameObjects(commandBuffer, m_GameObjects);
                m_Renderer.EndSwapChainRenderPass(commandBuffer);
                m_Renderer.EndFrame();
            }
        }

        vkDeviceWaitIdle(m_Device.GetDevice());
    }

    void FirstApp::LoadGameObjects()
    {
        std::vector<Liara_Model::Vertex> vertices{
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };
        const auto model = std::make_shared<Liara_Model>(m_Device, vertices);

        std::vector<glm::vec3> colors{
            {1.f, .7f, .73f},
            {1.f, .87f, .73f},
            {1.f, 1.f, .73f},
            {.73f, 1.f, .8f},
            {.73, .88f, 1.f}
        };

        for (auto& color : colors) { color = glm::pow(color, glm::vec3{2.2f}); }

        for (int i = 0; i < 40; i++)
        {
            auto triangle = Liara_GameObject::CreateGameObject();
            triangle.m_Model = model;
            triangle.m_Transform.scale = glm::vec2(.5f) + static_cast<float>(i) * 0.025f;
            triangle.m_Transform.rotation = static_cast<float>(i) * glm::pi<float>() * .025f;
            triangle.m_color = colors[i % colors.size()];
            m_GameObjects.push_back(std::move(triangle));
        }
    }
}