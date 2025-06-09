#pragma once

#include "Liara_Device.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace Liara::Graphics
{
    class Liara_SwapChain
    {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        Liara_SwapChain(Liara_Device &deviceRef, VkExtent2D windowExtent, const Core::SettingsManager &settings);
        Liara_SwapChain(Liara_Device &deviceRef, VkExtent2D windowExtent, const std::shared_ptr<Liara_SwapChain>& oldSwapChain);
        ~Liara_SwapChain();

        Liara_SwapChain(const Liara_SwapChain &) = delete;
        Liara_SwapChain &operator=(const Liara_SwapChain &) = delete;

        [[nodiscard]] VkFramebuffer GetFrameBuffer(const int index) const { return m_SwapChainFramebuffers[index]; }
        [[nodiscard]] VkRenderPass GetRenderPass() const { return m_RenderPass; }
        [[nodiscard]] VkImageView GetImageView(const int index) const { return m_SwapChainImageViews[index]; }
        [[nodiscard]] size_t ImageCount() const { return m_SwapChainImages.size(); }
        [[nodiscard]] VkFormat GetSwapChainImageFormat() const { return m_SwapChainImageFormat; }
        [[nodiscard]] VkExtent2D GetSwapChainExtent() const { return m_SwapChainExtent; }
        [[nodiscard]] uint32_t Width() const { return m_SwapChainExtent.width; }
        [[nodiscard]] uint32_t Height() const { return m_SwapChainExtent.height; }

        [[nodiscard]] float ExtentAspectRatio() const { return static_cast<float>(m_SwapChainExtent.width) / static_cast<float>(m_SwapChainExtent.height); }
        [[nodiscard]] VkFormat FindDepthFormat() const;

        VkResult AcquireNextImage(uint32_t *imageIndex) const;
        VkResult SubmitCommandBuffers(const VkCommandBuffer *buffers, const uint32_t *imageIndex);

        [[nodiscard]] bool CompareSwapFormat(const Liara_SwapChain& swap_chain) const
        {
            return swap_chain.m_SwapChainDepthFormat == m_SwapChainDepthFormat && swap_chain.m_SwapChainImageFormat == m_SwapChainImageFormat;
        }

    private:
        void Init();
        void CreateSwapChain();
        void CreateImageViews();
        void CreateDepthResources();
        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateSyncObjects();

        // Helper functions
        static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        [[nodiscard]] VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) const;
        [[nodiscard]] VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;

        const Core::SettingsManager &m_SettingsManager;

        VkFormat m_SwapChainImageFormat{};
        VkFormat m_SwapChainDepthFormat{};
        VkExtent2D m_SwapChainExtent{};

        std::vector<VkFramebuffer> m_SwapChainFramebuffers;
        VkRenderPass m_RenderPass{};

        std::vector<VkImage> m_DepthImages;
        std::vector<VkDeviceMemory> m_DepthImageMemorys;
        std::vector<VkImageView> m_DepthImageViews;
        std::vector<VkImage> m_SwapChainImages;
        std::vector<VkImageView> m_SwapChainImageViews;

        Liara_Device &m_Device;
        VkExtent2D m_WindowExtent;

        VkSwapchainKHR m_SwapChain{};
        std::shared_ptr<Liara_SwapChain> m_OldSwapChain;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;
        std::vector<VkFence> m_ImagesInFlight;
        size_t m_CurrentFrame = 0;
    };
}