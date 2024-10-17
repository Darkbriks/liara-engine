#pragma once

#include "Liara_Device.h"

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace Liara
{
    class Liara_SwapChain
    {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        Liara_SwapChain(Liara_Device &deviceRef, VkExtent2D windowExtent);
        ~Liara_SwapChain();

        Liara_SwapChain(const Liara_SwapChain &) = delete;
        void operator=(const Liara_SwapChain &) = delete;

        [[nodiscard]] VkFramebuffer GetFrameBuffer(int index) const { return m_SwapChainFramebuffers[index]; }
        [[nodiscard]] VkRenderPass GetRenderPass() const { return m_RenderPass; }
        [[nodiscard]] VkImageView GetImageView(int index) const { return m_SwapChainImageViews[index]; }
        [[nodiscard]] size_t ImageCount() const { return m_SwapChainImages.size(); }
        [[nodiscard]] VkFormat GetSwapChainImageFormat() const { return m_SwapChainImageFormat; }
        [[nodiscard]] VkExtent2D GetSwapChainExtent() const { return m_SwapChainExtent; }
        [[nodiscard]] uint32_t Width() const { return m_SwapChainExtent.width; }
        [[nodiscard]] uint32_t Height() const { return m_SwapChainExtent.height; }

        [[nodiscard]] float ExtentAspectRatio() const { return static_cast<float>(m_SwapChainExtent.width) / static_cast<float>(m_SwapChainExtent.height); }
        [[nodiscard]] VkFormat FindDepthFormat() const;

        VkResult AcquireNextImage(uint32_t *imageIndex);
        VkResult SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

    private:
        void CreateSwapChain();
        void CreateImageViews();
        void CreateDepthResources();
        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        [[nodiscard]] VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;

        VkFormat m_SwapChainImageFormat;
        VkExtent2D m_SwapChainExtent;

        std::vector<VkFramebuffer> m_SwapChainFramebuffers;
        VkRenderPass m_RenderPass;

        std::vector<VkImage> m_DepthImages;
        std::vector<VkDeviceMemory> m_DepthImageMemorys;
        std::vector<VkImageView> m_DepthImageViews;
        std::vector<VkImage> m_SwapChainImages;
        std::vector<VkImageView> m_SwapChainImageViews;

        Liara_Device &m_Device;
        VkExtent2D m_WindowExtent;

        VkSwapchainKHR m_SwapChain;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;
        std::vector<VkFence> m_ImagesInFlight;
        size_t m_CurrentFrame = 0;
    };
}  // namespace Liara
