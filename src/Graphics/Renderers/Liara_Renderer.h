#pragma once

#include "Graphics/Liara_Device.h"
#include "Graphics/Liara_SwapChain.h"
#include "Plateform/Liara_Window.h"

namespace Liara::Graphics::Renderers
{
    class Liara_Renderer
    {
    public:
        Liara_Renderer(Core::SettingsManager& settingsManager, Plateform::Liara_Window& window, Liara_Device& device)
            : m_SettingsManager(settingsManager), m_Window(window), m_Device(device) {}
        virtual ~Liara_Renderer() = default;
        Liara_Renderer(const Liara_Renderer&) = delete;
        Liara_Renderer& operator=(const Liara_Renderer&) = delete;

        [[nodiscard]] virtual VkRenderPass GetRenderPass() const = 0;
        [[nodiscard]] virtual uint32_t GetImageCount() const = 0;
        [[nodiscard]] virtual float GetAspectRatio() const = 0;
        [[nodiscard]] virtual uint32_t GetFrameIndex() const = 0;
        [[nodiscard]] virtual bool IsFrameInProgress() const = 0;
        [[nodiscard]] virtual VkCommandBuffer GetCurrentCommandBuffer() const = 0;

        virtual VkCommandBuffer BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void BeginRenderPass(VkCommandBuffer commandBuffer) const = 0;
        virtual void EndRenderPass(VkCommandBuffer commandBuffer) const = 0;

    protected:
        Core::SettingsManager& m_SettingsManager;
        Plateform::Liara_Window& m_Window;
        Liara_Device& m_Device;
    };
}