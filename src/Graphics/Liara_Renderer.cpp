#include "Liara_Renderer.h"

#include <array>
#include <stdexcept>

namespace Liara::Graphics
{
    Liara_Renderer::Liara_Renderer(Plateform::Liara_Window& window, Liara_Device& device)
    : m_Window(window), m_Device(device), m_CurrentImageIndex(0), m_CurrentFrameIndex(0), m_IsFrameStarted(false)
    {
        CreateSwapChain();
        CreateCommandBuffers();
    }

    Liara_Renderer::~Liara_Renderer()
    {
        FreeCommandBuffers();
    }

    VkCommandBuffer Liara_Renderer::BeginFrame()
    {
        assert(!m_IsFrameStarted && "Can't call BeginFrame while already in progress");
        const auto result = m_SwapChain->AcquireNextImage(&m_CurrentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            CreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to acquire swap chain image!");
        }

        m_IsFrameStarted = true;

        const auto commandBuffer = GetCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }
        return commandBuffer;
    }

    void Liara_Renderer::EndFrame()
    {
        assert(m_IsFrameStarted && "Can't call EndFrame while frame is not in progress");
        const auto commandBuffer = GetCurrentCommandBuffer();
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to record command buffer!");
        }

        if (const auto result = m_SwapChain->SubmitCommandBuffers(&commandBuffer, &m_CurrentImageIndex); result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window.WasResized())
        {
            m_Window.ResetResizedFlag();
            CreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to present swap chain image!");
        }

        m_IsFrameStarted = false;
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % Liara_SwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void Liara_Renderer::BeginSwapChainRenderPass(VkCommandBuffer command_buffer) const
    {
        assert(m_IsFrameStarted && "Can't call BeginSwapChainRenderPass if frame is not in progress");
        assert(command_buffer == GetCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_SwapChain->GetRenderPass();
        renderPassInfo.framebuffer = m_SwapChain->GetFrameBuffer(static_cast<int>(m_CurrentImageIndex));
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_SwapChain->GetSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_SwapChain->GetSwapChainExtent().width);
        viewport.height = static_cast<float>(m_SwapChain->GetSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        const VkRect2D scissor{{0, 0}, m_SwapChain->GetSwapChainExtent()};
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    }

    void Liara_Renderer::EndSwapChainRenderPass(VkCommandBuffer command_buffer) const
    {
        assert(m_IsFrameStarted && "Can't call EndSwapChainRenderPass if frame is not in progress");
        assert(command_buffer == GetCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");

        vkCmdEndRenderPass(command_buffer);
    }

    void Liara_Renderer::CreateCommandBuffers()
    {
        m_CommandBuffers.resize(Liara_SwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo commandBufferAllocInfo{};
        commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocInfo.commandPool = m_Device.GetCommandPool();
        commandBufferAllocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

        if (vkAllocateCommandBuffers(m_Device.GetDevice(), &commandBufferAllocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate command buffers!");
        }
    }

    void Liara_Renderer::FreeCommandBuffers()
    {
        vkFreeCommandBuffers(
            m_Device.GetDevice(),
            m_Device.GetCommandPool(),
            static_cast<uint32_t>(m_CommandBuffers.size()),
            m_CommandBuffers.data());
        m_CommandBuffers.clear();
    }

    void Liara_Renderer::CreateSwapChain()
    {
        auto extent = m_Window.GetExtent();
        while (extent.width == 0 || extent.height == 0)
        {
            SDL_WaitEvent(nullptr);
            extent = m_Window.GetExtent();
        }

        vkDeviceWaitIdle(m_Device.GetDevice());

        if (m_SwapChain == nullptr)
        {
            m_SwapChain = std::make_unique<Liara_SwapChain>(m_Device, extent);
        }
        else
        {
            std::shared_ptr<Liara_SwapChain> old_swap_chain = std::move(m_SwapChain);
            m_SwapChain = std::make_unique<Liara_SwapChain>(m_Device, extent, old_swap_chain);

            if (!old_swap_chain->CompareSwapFormat(*m_SwapChain))
            {
                throw std::runtime_error("Swap chain image or depth format has changed!");
            }
        }
    }
}