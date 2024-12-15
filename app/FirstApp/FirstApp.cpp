#include "FirstApp.h"
#include "Listener/KeybordMovementController.h"
#include "Systems/SimpleRenderSystem.h"
#include "Graphics/Liara_Buffer.h"
#include "Graphics/Ubo/GlobalUbo.h"

#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <chrono>
#include <iostream>
#include <numeric>

#include "Systems/PointLightSystem.h"

FirstApp::FirstApp()
{
    m_globalDescriptorPool = Liara::Graphics::Descriptors::Liara_DescriptorPool::Builder(m_Device)
                             .SetMaxSets(Liara::Graphics::Liara_SwapChain::MAX_FRAMES_IN_FLIGHT)
                             .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Liara::Graphics::Liara_SwapChain::MAX_FRAMES_IN_FLIGHT)
                             .Build();
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

    auto globalSetLayout = Liara::Graphics::Descriptors::Liara_DescriptorSetLayout::Builder(m_Device)
        .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .Build();

    std::vector<VkDescriptorSet> globalDescriptorSets(Liara::Graphics::Liara_SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < globalDescriptorSets.size(); i++)
    {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        Liara::Graphics::Descriptors::Liara_DescriptorWriter(*globalSetLayout, *m_globalDescriptorPool)
            .WriteBuffer(0, &bufferInfo)
            .Build(globalDescriptorSets[i]);
    }

    const Liara::Systems::SimpleRenderSystem render_system{m_Device, m_Renderer.GetSwapChainRenderPass(), globalSetLayout->GetDescriptorSetLayout()};
    const Liara::Systems::PointLightSystem pointLightSystem{m_Device, m_Renderer.GetSwapChainRenderPass(), globalSetLayout->GetDescriptorSetLayout()};

    Liara::Core::Liara_Camera camera {};

    auto player = Liara::Core::Liara_GameObject::CreateGameObject();
    player.m_Transform.position.z = -2.5f;
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
            Liara::Core::FrameInfo frameInfo{
                frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], m_GameObjects
            };

            // Update
            Liara::Graphics::Ubo::GlobalUbo ubo{};
            ubo.projection = camera.GetProjectionMatrix();
            ubo.view = camera.GetViewMatrix();
            ubo.inverseView = camera.GetInverseViewMatrix();
            pointLightSystem.Update(frameInfo, ubo);
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            if (auto result = uboBuffers[frameIndex]->flush(); result != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to flush buffer");
            }

            // Render
            m_Renderer.BeginSwapChainRenderPass(commandBuffer);
            render_system.RenderGameObjects(frameInfo);
            pointLightSystem.Render(frameInfo);
            m_Renderer.EndSwapChainRenderPass(commandBuffer);
            m_Renderer.EndFrame();
        }
    }

    vkDeviceWaitIdle(m_Device.GetDevice());
}

void FirstApp::LoadGameObjects()
{
    std::shared_ptr<Liara::Graphics::Liara_Model> model = Liara::Graphics::Liara_Model::CreateModelFromFile(m_Device, "assets/flat_vase.obj");
    auto flatVase = Liara::Core::Liara_GameObject::CreateGameObject();
    flatVase.m_Model = model;
    flatVase.m_Transform.position = {-.5f, .5f, 0.f};
    flatVase.m_Transform.scale = {3.f, 1.5f, 3.f};
    m_GameObjects.emplace(flatVase.GetId(), std::move(flatVase));

    model = Liara::Graphics::Liara_Model::CreateModelFromFile(m_Device, "assets/smooth_vase.obj");
    auto smoothVase = Liara::Core::Liara_GameObject::CreateGameObject();
    smoothVase.m_Model = model;
    smoothVase.m_Transform.position = {.5f, .5f, 0.f};
    smoothVase.m_Transform.scale = {3.f, 1.5f, 3.f};
    m_GameObjects.emplace(smoothVase.GetId(), std::move(smoothVase));

    model = Liara::Graphics::Liara_Model::CreateModelFromFile(m_Device, "assets/quad.obj");
    auto floor = Liara::Core::Liara_GameObject::CreateGameObject();
    floor.m_Model = model;
    floor.m_Transform.position = {0.f, .5f, 0.f};
    floor.m_Transform.scale = {3.f, 1.f, 3.f};
    m_GameObjects.emplace(floor.GetId(), std::move(floor));

    /*{
        auto pointLight = Liara::Core::Liara_GameObject::MakePointLight(0.2f);
        m_GameObjects.emplace(pointLight.GetId(), std::move(pointLight));
    }*/

    std::vector<glm::vec3> lightColors{
        {1.f, .1f, .1f},
        {.1f, .1f, 1.f},
        {.1f, 1.f, .1f},
        {1.f, 1.f, .1f},
        {.1f, 1.f, 1.f},
        //{1.f, 1.f, 1.f}
    };

    for (int i = 0; i < lightColors.size(); i++)
    {
        auto pointLight = Liara::Core::Liara_GameObject::MakePointLight(0.2f);
        pointLight.m_color = lightColors[i];
        auto rotateLight = glm::rotate(
            glm::mat4(1.f),
            (i * glm::two_pi<float>()) / lightColors.size(),
            {0.f, -1.f, 0.f});
        pointLight.m_Transform.position = glm::vec3(rotateLight * glm::vec4(-1.f, -0.3f, -1.f, 1.f));
        m_GameObjects.emplace(pointLight.GetId(), std::move(pointLight));
    }
}
