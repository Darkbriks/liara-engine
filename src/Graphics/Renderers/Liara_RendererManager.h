#pragma once

#include "Liara_Renderer.h"

#include <memory>

namespace Liara::Graphics::Renderers
{
    enum class RendererType
    {
        FORWARD,
    };

    class Liara_RendererManager
    {
    public:
        Liara_RendererManager(Plateform::Liara_Window& window, Liara_Device& device, Core::Liara_SettingsManager& settings_manager, RendererType type = RendererType::FORWARD);
        ~Liara_RendererManager();

        void SetRenderer(RendererType type);

        [[nodiscard]] RendererType GetRendererType() const { return m_RendererType; }
        [[nodiscard]]  const Liara_Renderer& GetRenderer() const { return *m_Renderer; }

        [[nodiscard]] VkCommandBuffer BeginFrame() const;
        void EndFrame() const;
        void BeginRenderPass(VkCommandBuffer commandBuffer) const;
        void EndRenderPass(VkCommandBuffer commandBuffer) const;

        void CleanUp();

    private:
        Core::Liara_SettingsManager& m_SettingsManager;
        Plateform::Liara_Window& m_Window;
        Liara_Device& m_Device;
        std::unique_ptr<Liara_Renderer> m_Renderer;
        RendererType m_RendererType;
    };
}