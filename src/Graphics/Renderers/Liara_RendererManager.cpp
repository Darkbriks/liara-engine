#include "Liara_RendererManager.h"

#include "Core/Liara_SettingsManager.h"
#include "Graphics/Liara_Device.h"
#include "Plateform/Liara_Window.h"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <memory>
#include <stdexcept>

#include "Liara_ForwardRenderer.h"

namespace Liara::Graphics::Renderers
{
    Liara_RendererManager::Liara_RendererManager(Plateform::Liara_Window& window,
                                                 Liara_Device& device,
                                                 Core::Liara_SettingsManager& settingsManager,
                                                 const RendererType type)
        : m_SettingsManager(settingsManager)
        , m_Window(window)
        , m_Device(device)
        , m_RendererType(type) {
        SetRenderer(type);
    }

    Liara_RendererManager::~Liara_RendererManager() { CleanUp(); }

    void Liara_RendererManager::SetRenderer(const RendererType type) {
        CleanUp();

        switch (type) {
            case RendererType::FORWARD:
                m_Renderer = std::make_unique<Liara_ForwardRenderer>(m_SettingsManager, m_Window, m_Device);
                break;
            default: LIARA_THROW_RUNTIME_ERROR(LogGraphics, "Invalid renderer type");
        }
        m_RendererType = type;
    }

    VkCommandBuffer Liara_RendererManager::BeginFrame() const {
        assert(m_Renderer && "Renderer not set!");
        return m_Renderer->BeginFrame();
    }

    void Liara_RendererManager::EndFrame() const {
        assert(m_Renderer && "Renderer not set!");
        m_Renderer->EndFrame();
    }

    void Liara_RendererManager::BeginRenderPass(VkCommandBuffer commandBuffer) const {
        assert(m_Renderer && "Renderer not set!");
        m_Renderer->BeginRenderPass(commandBuffer);
    }

    void Liara_RendererManager::EndRenderPass(VkCommandBuffer commandBuffer) const {
        assert(m_Renderer && "Renderer not set!");
        m_Renderer->EndRenderPass(commandBuffer);
    }

    void Liara_RendererManager::CleanUp() {
        if (m_Renderer) { m_Renderer.reset(); }
    }
}
