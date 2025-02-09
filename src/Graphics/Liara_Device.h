#pragma once

#include "Plateform/Liara_Window.h"

#include <string>
#include <vector>

namespace Liara::Graphics
{
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR m_Capabilities;
        std::vector<VkSurfaceFormatKHR> m_Formats;
        std::vector<VkPresentModeKHR> m_PresentModes;
    };

    struct QueueFamilyIndices
    {
        uint32_t m_GraphicsFamily;
        uint32_t m_PresentFamily;
        bool m_GraphicsFamilyHasValue = false;
        bool m_PresentFamilyHasValue = false;
        [[nodiscard]] bool IsComplete() const { return m_GraphicsFamilyHasValue && m_PresentFamilyHasValue; }
    };

    class Liara_Device
    {
    public:
        explicit Liara_Device(Plateform::Liara_Window &window);
        ~Liara_Device();

        // Not copyable or movable
        Liara_Device(const Liara_Device &) = delete;
        Liara_Device &operator=(const Liara_Device &) = delete;
        Liara_Device(Liara_Device &&) = delete;
        Liara_Device &operator=(Liara_Device &&) = delete;

        [[nodiscard]] VkCommandPool GetCommandPool() const { return m_CommandPool; }
        [[nodiscard]] VkDevice GetDevice() const { return m_Device; }
        [[nodiscard]] VkSurfaceKHR GetSurface() const { return m_Surface; }
        [[nodiscard]] VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
        [[nodiscard]] VkQueue GetPresentQueue() const { return m_PresentQueue; }
        [[nodiscard]] VkInstance GetInstance() const { return m_Instance; }
        [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
        [[nodiscard]] uint32_t GetGraphicsQueueFamily() const { return FindPhysicalQueueFamilies().m_GraphicsFamily; }

        SwapChainSupportDetails GetSwapChainSupport() const { return QuerySwapChainSupport(m_PhysicalDevice); }
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
        QueueFamilyIndices FindPhysicalQueueFamilies() const { return FindQueueFamilies(m_PhysicalDevice); }
        VkFormat FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

        // Buffer Helper Functions
        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory) const;
        VkCommandBuffer BeginSingleTimeCommands() const;
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer) const;
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) const;

        void CreateImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory) const;

        VkPhysicalDeviceProperties m_Properties{};

    private:
        void CreateInstance();
        void SetupDebugMessenger();
        void CreateSurface();
        void PickPhysicalDevice();
        void CreateLogicalDevice();
        void CreateCommandPool();

        // helper functions
        bool IsDeviceSuitable(VkPhysicalDevice device) const;
        std::vector<const char *> GetRequiredExtensions() const;
        bool CheckValidationLayerSupport() const;
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
        void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
        void HasSdl2RequiredInstanceExtensions() const;
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device) const;
        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;

        VkInstance m_Instance{};
        VkDebugUtilsMessengerEXT m_DebugMessenger{};
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        Plateform::Liara_Window &m_Window;
        VkCommandPool m_CommandPool{};

        VkDevice m_Device{};
        VkSurfaceKHR m_Surface{};
        VkQueue m_GraphicsQueue{};
        VkQueue m_PresentQueue{};

        const std::vector<const char *> m_ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char *> m_DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    };
}