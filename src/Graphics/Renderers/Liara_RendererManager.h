#pragma once

#include "Core/Liara_SettingsManager.h"
#include "Graphics/Liara_Device.h"
#include "Plateform/Liara_Window.h"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <memory>

#include "Liara_Renderer.h"

namespace Liara::Graphics::Renderers
{
    enum class RendererType : uint8_t
    {
        FORWARD,
    };

    class Liara_RendererManager
    {
    public:
        Liara_RendererManager(Plateform::Liara_Window& window,
                              Liara_Device& device,
                              Core::Liara_SettingsManager& settingsManager,
                              RendererType type = RendererType::FORWARD);
        ~Liara_RendererManager();

        void SetRenderer(RendererType type);

        [[nodiscard]] RendererType GetRendererType() const { return m_RendererType; }
        [[nodiscard]] const Liara_Renderer& GetRenderer() const { return *m_Renderer; }

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