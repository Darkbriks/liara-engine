//
// Created by antoi on 15/10/2024.
//

#include "FirstApp.h"
#include "CubeModel.h"
#include "../../Listener/KeybordMovementController.h"
#include "../../SimpleRenderSystem.h"

#include <array>
#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <chrono>

namespace Liara
{
    FirstApp::FirstApp()
    {
        LoadGameObjects();
    }

    void FirstApp::Run()
    {
        const SimpleRenderSystem render_system{m_Device, m_Renderer.GetSwapChainRenderPass()};

        Liara_Camera camera {};

        auto player = Liara_GameObject::CreateGameObject();
        KeybordMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!m_Window.ShouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(m_Window.GetWindow(), frameTime, player);
            camera.SetViewYXZ(player.m_Transform.position, player.m_Transform.rotation);

            const float aspect = m_Renderer.GetAspectRatio();
            //camera.SetOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
            camera.SetPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 100.0f);

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
