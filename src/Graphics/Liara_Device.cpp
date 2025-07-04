#include "Liara_Device.h"

#include "Core/Liara_SettingsManager.h"
#include "Plateform/Liara_Window.h"

#include <fmt/core.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_vulkan.h>
#include <set>
#include <stdexcept>
#include <unordered_set>
#include <vector>

namespace
{
    // local callback functions
    VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
                                                 VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
                                                 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                 void* /*pUserData*/) {
        fmt::print(stderr, "validation layer: {}\n", pCallbackData->pMessage);
        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator,
                                          VkDebugUtilsMessengerEXT* pDebugMessenger) {
        if (const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
            func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                       VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks* pAllocator) {
        if (const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
            func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }
}

namespace Liara::Graphics
{
    // class member functions
    Liara_Device::Liara_Device(Plateform::Liara_Window& window, Core::Liara_SettingsManager& settings)
        : m_SettingsManager(settings)
        , m_Window(window) {
        CreateInstance();
#ifndef NDEBUG
        SetupDebugMessenger();
#endif
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDevice();
        CreateCommandPool();
    }

    Liara_Device::~Liara_Device() {
        vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
        vkDestroyDevice(m_Device, nullptr);
        DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        vkDestroyInstance(m_Instance, nullptr);
    }

    void Liara_Device::CreateInstance() {
#ifndef NDEBUG
        if (!CheckValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }
#endif

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = m_SettingsManager.GetString("global.app_name").c_str();
        appInfo.applicationVersion = m_SettingsManager.GetUInt("global.app_version");
        appInfo.pEngineName = m_SettingsManager.GetString("global.engine_name").c_str();
        appInfo.engineVersion = m_SettingsManager.GetUInt("global.engine_version");
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        const auto extensions = GetRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();


#ifndef NDEBUG
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
        createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
#else
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
#endif

        if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }

        HasSdl2RequiredInstanceExtensions();
    }

    void Liara_Device::PickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
        if (deviceCount == 0) { throw std::runtime_error("failed to find GPUs with Vulkan support!"); }

        fmt::print("Device count: {}\n", deviceCount);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (IsDeviceSuitable(device)) {
                m_PhysicalDevice = device;
                break;
            }
        }

        if (m_PhysicalDevice == VK_NULL_HANDLE) { throw std::runtime_error("failed to find a suitable GPU!"); }

        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &deviceProperties);
        fmt::print("physical device: {}\n", deviceProperties.deviceName);
    }

    void Liara_Device::CreateLogicalDevice() {
        const QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        const std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

        constexpr float queuePriority = 1.0f;
        for (const uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy =
            m_SettingsManager.GetBool("texture.use_anisotropic_filtering") ? VK_TRUE : VK_FALSE;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &properties);
        m_SettingsManager.SetUInt("texture.max_anisotropy",
                                  std::min(static_cast<uint32_t>(properties.limits.maxSamplerAnisotropy),
                                           m_SettingsManager.GetUInt("texture.max_anisotropy")));

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

#ifndef NDEBUG
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
        createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
#else
        createInfo.enabledLayerCount = 0;
#endif

        if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(m_Device, indices.graphicsFamily, 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, indices.presentFamily, 0, &m_PresentQueue);
    }

    void Liara_Device::CreateCommandPool() {
        const QueueFamilyIndices queueFamilyIndices = FindPhysicalQueueFamilies();

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void Liara_Device::CreateSurface() { m_Window.CreateWindowSurface(m_Instance, &m_Surface); }

    bool Liara_Device::IsDeviceSuitable(VkPhysicalDevice device) const {
        const QueueFamilyIndices indices = FindQueueFamilies(device);

        const bool extensionsSupported = CheckDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            const SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        if (m_SettingsManager.GetBool("texture.use_anisotropic_filtering")
            && supportedFeatures.samplerAnisotropy == 0u) {
            fmt::print("Device does not support anisotropic filtering\n");
            return false;
        }

        if (m_SettingsManager.GetBool("texture.use_bindless_textures") && !CheckBindlessTextureSupport(device)) {
            fmt::print("Device does not support bindless textures\n");
            return false;
        }

        return indices.IsComplete() && extensionsSupported && swapChainAdequate;
    }

    void Liara_Device::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
    }

    void Liara_Device::SetupDebugMessenger() {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        PopulateDebugMessengerCreateInfo(createInfo);
        if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    bool Liara_Device::CheckValidationLayerSupport() const {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : m_ValidationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) { return false; }
        }

        return true;
    }

    bool Liara_Device::CheckBindlessTextureSupport(VkPhysicalDevice device) {
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexingFeatures = {};
        descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;

        VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = &descriptorIndexingFeatures;

        vkGetPhysicalDeviceFeatures2(device, &deviceFeatures2);

        if (descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount == VK_TRUE) { return true; }
        fmt::print("Device does not support bindless textures\n");
        return false;
    }

    std::vector<const char*> Liara_Device::GetRequiredExtensions() const {
        // Get the count
        uint32_t sdlExtensionCount = 0;
        if (SDL_Vulkan_GetInstanceExtensions(m_Window.GetWindow(), &sdlExtensionCount, nullptr) == 0u) {
            throw std::runtime_error("Failed to get SDL Vulkan extensions");
        }

        // Get the extensions
        auto* const sdlExtensions = new const char*[sdlExtensionCount];
        if (SDL_Vulkan_GetInstanceExtensions(m_Window.GetWindow(), &sdlExtensionCount, sdlExtensions) != SDL_TRUE) {
            throw std::runtime_error("Failed to get SDL Vulkan extensions");
        }

        std::vector<const char*> extensions(sdlExtensions, sdlExtensions + sdlExtensionCount);

#ifndef NDEBUG
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

        return extensions;
    }

    void Liara_Device::HasSdl2RequiredInstanceExtensions() const {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        fmt::print("available extensions:\n");
        std::unordered_set<std::string> available;
        for (const auto& [extensionName, specVersion] : extensions) {
            fmt::print("\t{}\n", extensionName);
            available.insert(extensionName);
        }

        fmt::print("required extensions:\n");
        const auto requiredExtensions = GetRequiredExtensions();
        for (const auto& required : requiredExtensions) {
            fmt::print("\t{}\n", required);
            if (!available.contains(required)) { throw std::runtime_error("Missing required glfw extension"); }
        }
    }

    bool Liara_Device::CheckDeviceExtensionSupport(VkPhysicalDevice device) const {
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

        for (const auto& [extensionName, specVersion] : availableExtensions) {
            requiredExtensions.erase(extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices Liara_Device::FindQueueFamilies(VkPhysicalDevice device) const {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u) {
                indices.graphicsFamily = i;
                indices.graphicsFamilyHasValue = true;
            }
            VkBool32 presentSupport = 0u;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
            if (queueFamily.queueCount > 0 && presentSupport != 0u) {
                indices.presentFamily = i;
                indices.presentFamilyHasValue = true;
            }
            if (indices.IsComplete()) { break; }

            i++;
        }

        return indices;
    }

    SwapChainSupportDetails Liara_Device::QuerySwapChainSupport(VkPhysicalDevice device) const {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                device, m_Surface, &presentModeCount, details.presentModes.data());
        }
        return details;
    }

    VkFormat Liara_Device::FindSupportedFormat(const std::vector<VkFormat>& candidates,
                                               VkImageTiling tiling,
                                               VkFormatFeatureFlags features) const {
        for (const VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        throw std::runtime_error("failed to find supported format!");
    }

    uint32_t Liara_Device::FindMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags properties) const {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if (((typeFilter & (1 << i)) != 0u)
                && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void Liara_Device::CreateBuffer(const VkDeviceSize size,
                                    const VkBufferUsageFlags usage,
                                    const VkMemoryPropertyFlags properties,
                                    VkBuffer& buffer,
                                    VkDeviceMemory& bufferMemory) const {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create vertex buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate vertex buffer memory!");
        }

        vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
    }

    VkCommandBuffer Liara_Device::BeginSingleTimeCommands() const {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_CommandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer = nullptr;
        vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    void Liara_Device::EndSingleTimeCommands(const VkCommandBuffer commandBuffer) const {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_GraphicsQueue);

        vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
    }

    void Liara_Device::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkDeviceSize size) const {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;  // Optional
        copyRegion.dstOffset = 0;  // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        EndSingleTimeCommands(commandBuffer);
    }

    void Liara_Device::CopyBufferToImage(
        VkBuffer buffer, VkImage image, const uint32_t width, const uint32_t height, const uint32_t layerCount) const {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layerCount;

        region.imageOffset = {.x = 0, .y = 0, .z = 0};
        region.imageExtent = {.width = width, .height = height, .depth = 1};

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        EndSingleTimeCommands(commandBuffer);
    }

    void Liara_Device::CreateImageWithInfo(const VkImageCreateInfo& imageInfo,
                                           const VkMemoryPropertyFlags properties,
                                           VkImage& image,
                                           VkDeviceMemory& imageMemory) const {
        if (vkCreateImage(m_Device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_Device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        if (vkBindImageMemory(m_Device, image, imageMemory, 0) != VK_SUCCESS) {
            throw std::runtime_error("failed to bind image memory!");
        }
    }
}