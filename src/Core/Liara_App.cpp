#include "Liara_App.h"

#include "Core/ApplicationInfo.h"
#include "Core/Liara_SignalHandler.h"
#include "Graphics/Descriptors/Liara_Descriptor.h"
#include "Graphics/GraphicsConstants.h"
#include "Graphics/Liara_Buffer.h"
#include "Graphics/Liara_Texture.h"
#include "Graphics/Ubo/GlobalUbo.h"
#include "Systems/ImGuiSystem.h"
#include "Systems/Liara_System.h"
#include "Systems/PointLightSystem.h"
#include "Systems/SimpleRenderSystem.h"

#include <vulkan/vulkan_core.h>

#include <memory>
#include <SDL2/SDL_events.h>
#include <stdexcept>

#include "FrameInfo.h"
#include "glm/trigonometric.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <backends/imgui_impl_sdl2.h>
#include <chrono>
#include <thread>


namespace Liara::Core
{
    Liara_App::Liara_App(const ApplicationInfo& appInfo)
        : m_ApplicationInfo(appInfo)
        , m_SettingsManager(std::make_unique<Liara_SettingsManager>(appInfo))
        , m_Window(*m_SettingsManager)
        , m_Device(m_Window, *m_SettingsManager)
        , m_RendererManager(m_Window, m_Device, *m_SettingsManager) {
        m_SettingsManager->LoadFromFile("settings.cfg");

        // Todo: Check if this is the right place to put this
        m_DescriptorAllocator =
            Graphics::Descriptors::Liara_DescriptorAllocator::Builder(m_Device)
                .SetMaxSets(Graphics::Constants::MAX_FRAMES_IN_FLIGHT)
                .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Graphics::Constants::MAX_FRAMES_IN_FLIGHT)
                .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Graphics::Constants::MAX_FRAMES_IN_FLIGHT)
                .Build();

        m_DescriptorLayoutCache = Graphics::Descriptors::Liara_DescriptorLayoutCache::Builder(m_Device).Build();
    }

    void Liara_App::Run() {
        // TODO: Test texture, temporary
        auto texture = Liara::Graphics::Liara_Texture::Builder{};
        texture.LoadTexture("assets/textures/viking_room.png", *m_SettingsManager);
        m_Texture = std::make_unique<Graphics::Liara_Texture>(m_Device, texture, *m_SettingsManager);

        Init();

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!m_Window.ShouldClose() && !Liara_SignalHandler::ShouldExit()) {
            frameStats.Reset();
            auto newTime = std::chrono::high_resolution_clock::now();
            const float frameTime = std::chrono::duration<float>(newTime - currentTime).count();
            currentTime = newTime;

            MasterProcessInput(frameTime);

            if (m_Window.WasMinimized()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            const float aspect = m_RendererManager.GetRenderer().GetAspectRatio();
            SetProjection(aspect);

            if (auto* const commandBuffer = m_RendererManager.BeginFrame()) {
                Systems::ImGuiSystem::NewFrame();

                const int frameIndex = static_cast<int>(m_RendererManager.GetRenderer().GetFrameIndex());
                const FrameInfo frameInfo{.frameIndex = frameIndex,
                                          .deltaTime = frameTime,
                                          .commandBuffer = commandBuffer,
                                          .camera = m_Camera,
                                          .globalDescriptorSet = m_GlobalDescriptorSets[frameIndex],
                                          .gameObjects = m_GameObjects};

                MasterUpdate(frameInfo);
                MasterRender(frameInfo);
                m_RendererManager.EndFrame();
            }
        }

        Close();
    }

    void Liara_App::Init() {
        InitSignalHandling();
        InitUboBuffers();
        InitDescriptorSets();
        InitSystems();
        InitCamera();
        LateInit();
    }

    void Liara_App::InitSignalHandling() {
        if (!Liara_SignalHandler::Initialize([this]() {})) {
            fmt::print(stderr, "Warning: Failed to initialize signal handling\n");
        }
    }

    void Liara_App::InitUboBuffers() {
        m_UboBuffers.resize(Graphics::Constants::MAX_FRAMES_IN_FLIGHT);
        m_UboMappings.reserve(Graphics::Constants::MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < Graphics::Constants::MAX_FRAMES_IN_FLIGHT; ++i) {
            m_UboBuffers[i] = std::make_unique<Graphics::Liara_Buffer>(
                m_Device, sizeof(Graphics::Ubo::GlobalUbo), Graphics::BufferConfig::Uniform());

            m_UboMappings.emplace_back(m_UboBuffers[i]->CreateMappingGuard());
        }
    }

    void Liara_App::InitDescriptorSets() {
        m_GlobalSetLayout = VkDescriptorSetLayout{};
        m_GlobalDescriptorSets.resize(Graphics::Constants::MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < m_GlobalDescriptorSets.size(); i++) {
            auto bufferInfo = m_UboBuffers[i]->DescriptorInfo();
            auto textureInfo = m_Texture->GetDescriptorInfo();
            Graphics::Descriptors::Liara_DescriptorBuilder(*m_DescriptorLayoutCache, *m_DescriptorAllocator)
                .BindBuffer(0, &bufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .BindImage(1, &textureInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .Build(m_GlobalDescriptorSets[i], m_GlobalSetLayout);
        }
    }

    void Liara_App::InitSystems() {
        m_Systems.push_back(std::make_unique<Systems::SimpleRenderSystem>(
            m_Device, m_RendererManager.GetRenderer().GetRenderPass(), m_GlobalSetLayout, *m_SettingsManager));
        m_Systems.push_back(std::make_unique<Systems::PointLightSystem>(
            m_Device, m_RendererManager.GetRenderer().GetRenderPass(), m_GlobalSetLayout, *m_SettingsManager));
        m_Systems.push_back(std::make_unique<Systems::ImGuiSystem>(m_Window,
                                                                   m_Device,
                                                                   m_RendererManager.GetRenderer().GetRenderPass(),
                                                                   m_RendererManager.GetRenderer().GetImageCount()));
    }

    void Liara_App::InitCamera() { m_Camera = Liara_Camera{}; }

    void Liara_App::SetProjection(const float aspect) {
        // camera.SetOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
        m_Camera.SetPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 100.0f);
    }

    void Liara_App::Close() {
        Liara_SignalHandler::Cleanup();
        vkDeviceWaitIdle(m_Device.GetDevice());
    }

    void Liara_App::MasterProcessInput(const float frameTime) {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) { ImGui_ImplSDL2_ProcessEvent(&event); }

        ProcessInput(frameTime);
    }

    void Liara_App::MasterUpdate(const FrameInfo& frameInfo) {
        Graphics::Ubo::GlobalUbo ubo(
            m_Camera.GetProjectionMatrix(), m_Camera.GetViewMatrix(), m_Camera.GetInverseViewMatrix());

        Update(frameInfo);
        for (const auto& system : m_Systems) { system->Update(frameInfo, ubo); }

        const auto& currentBuffer = m_UboBuffers[frameInfo.frameIndex];
        currentBuffer->WriteObject(ubo);

        if (!(currentBuffer->GetMemoryPropertyFlags() & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
            if (const auto result = currentBuffer->Flush(); result != VK_SUCCESS) {
                throw std::runtime_error("Failed to flush UBO buffer");
            }
        }
    }

    void Liara_App::MasterRender(const FrameInfo& frameInfo) {
        m_RendererManager.BeginRenderPass(frameInfo.commandBuffer);

        Render(frameInfo);
        for (const auto& system : m_Systems) { system->Render(frameInfo); }

        m_RendererManager.EndRenderPass(frameInfo.commandBuffer);
    }
}