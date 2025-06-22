/**
 * @file Liara_Device.h
 * @brief Defines the `Liara_Device` class, which encapsulates Vulkan device management.
 *
 * This class is responsible for creating and managing a Vulkan logical device, command pool, and surface
 * as well as providing helper functions for buffer and image creation, and swap chain support.
 */

#pragma once
#include "Core/Liara_SettingsManager.h"
#include "Plateform/Liara_Window.h"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <SDL2/SDL_vulkan.h>
#include <vector>

namespace Liara::Graphics
{
    /**
     * @struct SwapChainSupportDetails
     * @brief Structure holding the swap chain support details for a Vulkan physical device.
     */
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;       ///< Surface capabilities
        std::vector<VkSurfaceFormatKHR> formats;     ///< Surface formats
        std::vector<VkPresentModeKHR> presentModes;  ///< Present modes
    };

    /**
     * @struct QueueFamilyIndices
     * @brief Structure holding the indices of the graphics and present families for Vulkan queues.
     */
    struct QueueFamilyIndices
    {
        uint32_t graphicsFamily{};            ///< Graphics family index
        uint32_t presentFamily{};             ///< Present family index
        bool graphicsFamilyHasValue = false;  ///< Graphics family has value
        bool presentFamilyHasValue = false;   ///< Present family has value

        /**
         * @brief Checks if the queue families are complete (i.e., valid).
         * @return true if both graphics and present family indices are valid.
         */
        [[nodiscard]] bool IsComplete() const { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

    /**
     * @class Liara_Device
     * @brief Class that manages Vulkan device creation, command pool, surface, and other related operations.
     *
     * This class handles device initialization, memory management, and creation of Vulkan buffers and images.
     */
    class Liara_Device
    {
    public:
        /**
         * @brief Constructor to create a Vulkan logical device based on the window provided.
         * @param window Reference to a `Plateform::Liara_Window` object.
         * @param settings Reference to a `Core::Liara_SettingsContext` object containing graphics settings.
         */
        explicit Liara_Device(Plateform::Liara_Window& window, Core::Liara_SettingsManager& settings);

        /**
         * @brief Destructor that cleans up Vulkan resources.
         */
        ~Liara_Device();

        // Not copyable or movable
        Liara_Device(const Liara_Device&) = delete;
        Liara_Device& operator=(const Liara_Device&) = delete;
        Liara_Device(Liara_Device&&) = delete;
        Liara_Device& operator=(Liara_Device&&) = delete;

        [[nodiscard]] VkCommandPool GetCommandPool() const { return m_CommandPool; }
        [[nodiscard]] VkDevice GetDevice() const { return m_Device; }
        [[nodiscard]] VkSurfaceKHR GetSurface() const { return m_Surface; }
        [[nodiscard]] VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
        [[nodiscard]] VkQueue GetPresentQueue() const { return m_PresentQueue; }
        [[nodiscard]] VkInstance GetInstance() const { return m_Instance; }
        [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
        [[nodiscard]] uint32_t GetGraphicsQueueFamily() const { return FindPhysicalQueueFamilies().graphicsFamily; }

        /**
         * @brief Retrieves swap chain support details for the physical device.
         * @return A `SwapChainSupportDetails` structure containing swap chain support info.
         */
        [[nodiscard]] SwapChainSupportDetails GetSwapChainSupport() const {
            return QuerySwapChainSupport(m_PhysicalDevice);
        }

        /**
         * @brief Finds a suitable memory type for a given memory property.
         * @param typeFilter Filter to select memory type
         * @param properties Memory properties (e.g., GPU visible, device local)
         * @return The index of the suitable memory type.
         */
        [[nodiscard]] uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

        /**
         * @brief Finds queue families for the physical device.
         * @return A `QueueFamilyIndices` structure with indices for graphics and present families.
         */
        [[nodiscard]] QueueFamilyIndices FindPhysicalQueueFamilies() const {
            return FindQueueFamilies(m_PhysicalDevice);
        }

        /**
         * @brief Finds a supported Vulkan format from a list of candidates.
         * @param candidates List of possible formats to check.
         * @param tiling Image tiling mode (linear or optimal).
         * @param features Format features (e.g., color attachment).
         * @return The chosen supported format.
         */
        [[nodiscard]] VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates,
                                                   VkImageTiling tiling,
                                                   VkFormatFeatureFlags features) const;

        // Buffer Helper Functions
        /**
         * @brief Creates a Vulkan buffer with specified size and usage.
         * @param size The size of the buffer.
         * @param usage The buffer usage flags.
         * @param properties The memory properties for the buffer.
         * @param buffer The buffer to be created.
         * @param bufferMemory The memory to be allocated for the buffer.
         */
        void CreateBuffer(VkDeviceSize size,
                          VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties,
                          VkBuffer& buffer,
                          VkDeviceMemory& bufferMemory) const;

        /**
         * @brief Begins a single-time command buffer for immediate commands.
         * @return The created command buffer.
         */
        [[nodiscard]] VkCommandBuffer BeginSingleTimeCommands() const;

        /**
         * @brief Ends the single-time command buffer and submits the command.
         * @param commandBuffer The command buffer to submit.
         */
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer) const;

        /**
         * @brief Copies data from one buffer to another.
         * @param srcBuffer The source buffer.
         * @param dstBuffer The destination buffer.
         * @param size The size of data to copy.
         */
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;

        /**
         * @brief Copies data from a buffer to an image.
         * @param buffer The buffer containing the data.
         * @param image The image to copy the data to.
         * @param width The width of the image.
         * @param height The height of the image.
         * @param layerCount The number of layers in the image.
         */
        void
        CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) const;

        /**
         * @brief Creates an image with specified properties.
         * @param imageInfo The Vulkan image creation info structure.
         * @param properties The desired memory properties for the image.
         * @param image The created image.
         * @param imageMemory The allocated memory for the image.
         */
        void CreateImageWithInfo(const VkImageCreateInfo& imageInfo,
                                 VkMemoryPropertyFlags properties,
                                 VkImage& image,
                                 VkDeviceMemory& imageMemory) const;

        VkPhysicalDeviceProperties deviceProperties{};  ///< Physical device properties

    private:
        /**
         * @brief Creates the Vulkan instance.
         */
        void CreateInstance();

        /**
         * @brief Sets up Vulkan debug messenger for validation layers.
         */
        void SetupDebugMessenger();

        /**
         * @brief Creates a Vulkan surface based on the provided window.
         */
        void CreateSurface();

        /**
         * @brief Selects a suitable Vulkan physical device.
         */
        void PickPhysicalDevice();

        /**
         * @brief Creates a Vulkan logical device and its queues.
         */
        void CreateLogicalDevice();

        /**
         * @brief Creates a Vulkan command pool.
         */
        void CreateCommandPool();

        // helper functions
        /**
         * @brief Checks if a Vulkan device is suitable for the application.
         * @param device The Vulkan physical device to check.
         * @return true if the device meets the requirements.
         */
        bool IsDeviceSuitable(VkPhysicalDevice device) const;

        /**
         * @brief Gets the required Vulkan instance extensions.
         * @return A vector of required extension names.
         */
        [[nodiscard]] std::vector<const char*> GetRequiredExtensions() const;

        /**
         * @brief Checks if the Vulkan validation layers are supported.
         * @return true if validation layers are available.
         */
        [[nodiscard]] bool CheckValidationLayerSupport() const;

        /**
         * @brief Checks if the Vulkan physical device supports bindless textures.
         * @param device The physical device to check.
         * @return true if the device supports bindless textures.
         */
        static bool CheckBindlessTextureSupport(VkPhysicalDevice device);

        /**
         * @brief Finds the queue families for a given Vulkan physical device.
         * @param device The physical device.
         * @return A `QueueFamilyIndices` structure with family indices.
         */
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;

        /**
         * @brief Populates the debug messenger creation info structure.
         * @param createInfo The structure to be populated.
         */
        static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        /**
         * @brief Checks if SDL2-specific extensions are required for the Vulkan instance.
         */
        void HasSdl2RequiredInstanceExtensions() const;

        /**
         * @brief Checks if a Vulkan physical device supports the required device extensions.
         * @param device The physical device to check.
         * @return true if the device supports the necessary extensions.
         */
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device) const;

        /**
         * @brief Queries swap chain support details for a given physical device.
         * @param device The physical device.
         * @return Swap chain support details for the device.
         */
        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;

        Core::Liara_SettingsManager& m_SettingsManager;

        // Vulkan handles
        VkInstance m_Instance{};                             ///< Vulkan instance
        VkDebugUtilsMessengerEXT m_DebugMessenger{};         ///< Vulkan debug messenger
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;  ///< Vulkan physical device
        Plateform::Liara_Window& m_Window;                   ///< Reference to the window
        VkCommandPool m_CommandPool{};                       ///< Vulkan command pool

        VkDevice m_Device{};        ///< Vulkan logical device
        VkSurfaceKHR m_Surface{};   ///< Vulkan surface
        VkQueue m_GraphicsQueue{};  ///< Vulkan graphics queue
        VkQueue m_PresentQueue{};   ///< Vulkan present queue

        // Validation layers and device extensions required by the application
        const std::vector<const char*> m_ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char*> m_DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    };
}
