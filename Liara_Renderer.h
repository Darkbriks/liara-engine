//
// Created by antoi on 20/10/2024.
//

#pragma once
#include <cassert>
#include <memory>

#include "Liara_Device.h"
#include "Liara_SwapChain.h"
#include "Liara_Window.h"

namespace Liara
{
    class Liara_Renderer
    {
    public:
        Liara_Renderer(Liara_Window& window, Liara_Device& device);
        ~Liara_Renderer();
        Liara_Renderer(const Liara_Renderer&) = delete;
        Liara_Renderer& operator=(const Liara_Renderer&) = delete;

        [[nodiscard]] VkRenderPass GetSwapChainRenderPass() const { return  m_SwapChain->GetRenderPass(); }
        [[nodiscard]] float GetAspectRatio() const { return m_SwapChain->ExtentAspectRatio(); }
        [[nodiscard]] uint32_t GetFrameIndex() const
        {
            assert(m_IsFrameStarted && "Cannot get frame index when frame not in progress");
            return m_CurrentFrameIndex;
        }
        [[nodiscard]] bool IsFrameInProgress() const { return m_IsFrameStarted; }
        [[nodiscard]] VkCommandBuffer GetCurrentCommandBuffer() const
        {
            assert(m_IsFrameStarted && "Cannot get command buffer when frame not in progress");
            return m_CommandBuffers[m_CurrentFrameIndex];
        }

        VkCommandBuffer BeginFrame();
        void EndFrame();
        void BeginSwapChainRenderPass(VkCommandBuffer command_buffer) const;
        void EndSwapChainRenderPass(VkCommandBuffer command_buffer) const;

    private:
        void CreateCommandBuffers();
        void FreeCommandBuffers();
        void CreateSwapChain();

        Liara_Window& m_Window;
        Liara_Device& m_Device;
        std::unique_ptr<Liara_SwapChain> m_SwapChain;
        std::vector<VkCommandBuffer> m_CommandBuffers;

        uint32_t m_CurrentImageIndex;
        uint32_t m_CurrentFrameIndex;
        bool m_IsFrameStarted;
    };
} // Liara
