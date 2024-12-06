#include "FirstApp.h"
#include "Listener/KeybordMovementController.h"
#include "Systems/SimpleRenderSystem.h"
#include "Graphics/Liara_Buffer.h"
#include "Graphics/Ubo/GlobalUbo.h"

#include <array>
#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <chrono>
#include <numeric>

FirstApp::FirstApp()
{
    LoadGameObjects();
}

void FirstApp::Run()
{
    std::vector<std::unique_ptr<Liara::Graphics::Liara_Buffer>> uboBuffers(Liara::Graphics::Liara_SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (auto &uboBuffer : uboBuffers)
    {
        uboBuffer = std::make_unique<Liara::Graphics::Liara_Buffer>(
            m_Device,
            sizeof(Liara::Graphics::Ubo::GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        uboBuffer->map();
    }

    const Liara::Systems::SimpleRenderSystem render_system{m_Device, m_Renderer.GetSwapChainRenderPass()};

    Liara::Core::Liara_Camera camera {};

    auto player = Liara::Core::Liara_GameObject::CreateGameObject();
    Liara::Listener::KeybordMovementController cameraController{};

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
            const int frameIndex = static_cast<int>(m_Renderer.GetFrameIndex());
            Liara::Core::FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera};

            // Update
            Liara::Graphics::Ubo::GlobalUbo ubo{};
            ubo.projectionView = camera.GetProjectionMatrix() * camera.GetViewMatrix();
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            // Render
            m_Renderer.BeginSwapChainRenderPass(commandBuffer);
            render_system.RenderGameObjects(frameInfo, m_GameObjects);
            m_Renderer.EndSwapChainRenderPass(commandBuffer);
            m_Renderer.EndFrame();
        }
    }

    vkDeviceWaitIdle(m_Device.GetDevice());
}

void FirstApp::LoadGameObjects()
{
    const std::shared_ptr<Liara::Graphics::Liara_Model> model = Liara::Graphics::Liara_Model::CreateModelFromFile(m_Device, "assets/smooth_vase.obj");
    auto cube = Liara::Core::Liara_GameObject::CreateGameObject();
    cube.m_Model = model;
    cube.m_Transform.position = {0.0f, 0.5f, 2.5f};
    cube.m_Transform.scale = glm::vec3(3.0f);
    m_GameObjects.push_back(std::move(cube));
}
