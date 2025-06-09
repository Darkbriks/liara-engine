#include "Liara_RendererManager.h"
#include "Liara_ForwardRenderer.h"

#include <stdexcept>

namespace Liara::Graphics::Renderers
{
    Liara_RendererManager::Liara_RendererManager(Plateform::Liara_Window& window, Liara_Device& device, Core::SettingsManager& settings_manager, const RendererType type)
        : m_SettingsManager(settings_manager), m_Window(window), m_Device(device), m_RendererType(type)
    {
        SetRenderer(type);
    }

    Liara_RendererManager::~Liara_RendererManager()
    {
        CleanUp();
    }

    void Liara_RendererManager::SetRenderer(const RendererType type)
    {
        CleanUp();

        switch (type)
        {
            case RendererType::FORWARD:
                m_Renderer = std::make_unique<Liara_ForwardRenderer>(m_SettingsManager, m_Window, m_Device);
                break;
            default:
                throw std::runtime_error("Invalid renderer type");
        }
        m_RendererType = type;
    }

    VkCommandBuffer Liara_RendererManager::BeginFrame() const
    {
        assert(m_Renderer && "Renderer not set!");
        return m_Renderer->BeginFrame();
    }

    void Liara_RendererManager::EndFrame() const
    {
        assert(m_Renderer && "Renderer not set!");
        m_Renderer->EndFrame();
    }

    void Liara_RendererManager::BeginRenderPass(VkCommandBuffer commandBuffer) const
    {
        assert(m_Renderer && "Renderer not set!");
        m_Renderer->BeginRenderPass(commandBuffer);
    }

    void Liara_RendererManager::EndRenderPass(VkCommandBuffer commandBuffer) const
    {
        assert(m_Renderer && "Renderer not set!");
        m_Renderer->EndRenderPass(commandBuffer);
    }

    void Liara_RendererManager::CleanUp()
    {
        if (m_Renderer)
        {
            m_Renderer.reset();
        }
    }
}
