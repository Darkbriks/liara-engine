#pragma once

#include "Liara_Renderer.h"

#include <cassert>
#include <memory>

namespace Liara::Graphics::Renderers
{
    class Liara_ForwardRenderer final : public Liara_Renderer
    {
    public:
        Liara_ForwardRenderer(Core::Liara_SettingsManager& settingsManager, Plateform::Liara_Window& window, Liara_Device& device);
        ~Liara_ForwardRenderer() override;

        [[nodiscard]] VkRenderPass GetRenderPass() const override { return m_SwapChain->GetRenderPass(); }
        [[nodiscard]] uint32_t GetImageCount() const override { return static_cast<uint32_t>(m_SwapChain->ImageCount()); }
        [[nodiscard]] float GetAspectRatio() const override { return m_SwapChain->ExtentAspectRatio(); }
        [[nodiscard]] uint32_t GetFrameIndex() const override
        {
            assert(m_IsFrameStarted && "Cannot get frame index when frame not in progress");
            return m_CurrentFrameIndex;
        }
        [[nodiscard]] bool IsFrameInProgress() const override { return m_IsFrameStarted; }
        [[nodiscard]] VkCommandBuffer GetCurrentCommandBuffer() const override
        {
            assert(m_IsFrameStarted && "Cannot get command buffer when frame not in progress");
            return m_CommandBuffers[m_CurrentFrameIndex];
        }

        VkCommandBuffer BeginFrame() override;
        void EndFrame() override;
        void BeginRenderPass(VkCommandBuffer commandBuffer) const override;
        void EndRenderPass(VkCommandBuffer commandBuffer) const override;

    private:
        void CreateCommandBuffers();
        void FreeCommandBuffers();
        void CreateSwapChain();

        std::unique_ptr<Liara_SwapChain> m_SwapChain;
        std::vector<VkCommandBuffer> m_CommandBuffers;

        uint32_t m_CurrentImageIndex{};
        uint32_t m_CurrentFrameIndex{};
        bool m_IsFrameStarted{false};

        bool m_NeedsSwapChainRecreation{false};
        bool m_FullscreenChanged{false};
        bool m_VsyncState{false};
        bool m_VsyncChanged{false};
    };
}