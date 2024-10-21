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

#include "CubeModel.h"

namespace Liara
{
    FirstApp::FirstApp()
    {
        LoadGameObjects();
    }

    void FirstApp::Run()
    {
        Liara_Camera camera {};
        const SimpleRenderSystem render_system{m_Device, m_Renderer.GetSwapChainRenderPass()};

        while (!m_Window.ShouldClose())
        {
            glfwPollEvents();
            const float aspect = m_Renderer.GetAspectRatio();
            //camera.SetOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
            camera.SetPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 10.0f);

            if (const auto commandBuffer = m_Renderer.BeginFrame())
            {
                m_Renderer.BeginSwapChainRenderPass(commandBuffer);
                render_system.RenderGameObjects(commandBuffer, m_GameObjects, camera);
                m_Renderer.EndSwapChainRenderPass(commandBuffer);
                m_Renderer.EndFrame();
            }
        }

        vkDeviceWaitIdle(m_Device.GetDevice());
    }

    void FirstApp::LoadGameObjects()
    {
        const std::shared_ptr<Liara_Model> model = CubeModel::CreateCubeModel(m_Device, {0.0f, 0.0f, 0.0f});
        auto cube = Liara_GameObject::CreateGameObject();
        cube.m_Model = model;
        cube.m_Transform.position = {0.0f, 0.0f, 2.5f};
        cube.m_Transform.scale = {0.5f, 0.5f, 0.5f};
        m_GameObjects.push_back(std::move(cube));
    }
}
