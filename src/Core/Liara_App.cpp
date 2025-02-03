#include "Liara_App.h"

#include "FrameInfo.h"
#include "Graphics/Ubo/GlobalUbo.h"
#include "Systems/Liara_System.h"
#include "Systems/PointLightSystem.h"
#include "Systems/SimpleRenderSystem.h"
#include "Systems/ImGuiSystem.h"
#include "Graphics/Liara_SwapChain.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <chrono>
#include <iostream>
#include <imgui.h>

namespace Liara::Core
{
    Liara_App::Liara_App(std::string title, const unsigned short width, const unsigned short height)
        : TITLE(std::move(title)), WIDTH(width), HEIGHT(height),
          m_Window(TITLE, WIDTH, HEIGHT), m_Device(m_Window), m_Renderer(m_Window, m_Device)
    {
        // Todo: Check if this is the right place to put this
        m_DescriptorAllocator = Graphics::Descriptors::Liara_DescriptorAllocator::Builder(m_Device)
                                .SetMaxSets(Graphics::Liara_SwapChain::MAX_FRAMES_IN_FLIGHT)
                                .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                             Graphics::Liara_SwapChain::MAX_FRAMES_IN_FLIGHT)
                                .Build();

        m_DescriptorLayoutCache = Graphics::Descriptors::Liara_DescriptorLayoutCache::Builder(m_Device).Build();
    }

    void Liara_App::Run()
    {
        Init();

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!m_Window.ShouldClose())
        {
            g_FrameStats.Reset();

            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            const float frameTime = std::chrono::duration<float>(newTime - currentTime).count();
            currentTime = newTime;

            MasterProcessInput(frameTime);

            const float aspect = m_Renderer.GetAspectRatio();
            SetProjection(aspect);

            if (const auto commandBuffer = m_Renderer.BeginFrame())
            {
                Systems::ImGuiSystem::NewFrame();

                const int frameIndex = static_cast<int>(m_Renderer.GetFrameIndex());
                FrameInfo frameInfo{
                    frameIndex, frameTime, commandBuffer, m_Camera, m_GlobalDescriptorSets[frameIndex], m_GameObjects
                };

                MasterUpdate(frameInfo);
                MasterRender(commandBuffer, frameInfo);
                m_Renderer.EndFrame();
            }
        }

        Close();
    }

    void Liara_App::Init()
    {
        InitUboBuffers();
        InitDescriptorSets();
        InitSystems();
        InitCamera();
        LateInit();
    }

    void Liara_App::InitUboBuffers()
    {
        m_UboBuffers.resize(Graphics::Liara_SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto &uboBuffer : m_UboBuffers)
        {
            uboBuffer = std::make_unique<Graphics::Liara_Buffer>(
                m_Device,
                sizeof(Graphics::Ubo::GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            uboBuffer->map();
        }
    }

    void Liara_App::InitDescriptorSets()
    {
        m_GlobalSetLayout = VkDescriptorSetLayout{};
        m_GlobalDescriptorSets.resize(Graphics::Liara_SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < m_GlobalDescriptorSets.size(); i++)
        {
            auto bufferInfo = m_UboBuffers[i]->descriptorInfo();
            Graphics::Descriptors::Liara_DescriptorBuilder(*m_DescriptorLayoutCache, *m_DescriptorAllocator)
                .BindBuffer(0, &bufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .Build(m_GlobalDescriptorSets[i], m_GlobalSetLayout);
        }
    }

    void Liara_App::InitSystems()
    {
        m_Systems.push_back(std::make_unique<Systems::SimpleRenderSystem>(m_Device, m_Renderer.GetSwapChainRenderPass(), m_GlobalSetLayout));
        m_Systems.push_back(std::make_unique<Systems::PointLightSystem>(m_Device, m_Renderer.GetSwapChainRenderPass(), m_GlobalSetLayout));
        m_Systems.push_back(std::make_unique<Systems::ImGuiSystem>(m_Window, m_Device, m_Renderer.GetSwapChainRenderPass(), m_Renderer.GetImageCount()));
    }

    void Liara_App::InitCamera()
    {
        m_Camera = Liara_Camera{};
    }

    void Liara_App::SetProjection(const float aspect)
    {
        //camera.SetOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
        m_Camera.SetPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 100.0f);
    }

    void Liara_App::Close() { vkDeviceWaitIdle(m_Device.GetDevice()); }

    void Liara_App::MasterProcessInput(const float frameTime) { ProcessInput(frameTime); }

    void Liara_App::MasterUpdate(const FrameInfo& frameInfo)
    {
        // Todo: Check if this is the right place to put this
        Graphics::Ubo::GlobalUbo ubo{};
        ubo.projection = m_Camera.GetProjectionMatrix();
        ubo.view = m_Camera.GetViewMatrix();
        ubo.inverseView = m_Camera.GetInverseViewMatrix();

        Update(frameInfo);
        for (const auto& system: m_Systems) { system->Update(frameInfo, ubo); }

        m_UboBuffers[frameInfo.m_FrameIndex]->writeToBuffer(&ubo);
        if (const auto result = m_UboBuffers[frameInfo.m_FrameIndex]->flush(); result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to flush buffer");
        }
    }

    void Liara_App::MasterRender(VkCommandBuffer commandBuffer, const FrameInfo& frameInfo)
    {
        m_Renderer.BeginSwapChainRenderPass(commandBuffer);

        Render(frameInfo);
        for (const auto& system: m_Systems) { system->Render(frameInfo); }

        m_Renderer.EndSwapChainRenderPass(commandBuffer);
    }

}
