#pragma once

#include "Core/Liara_SettingsManager.h"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <stb/stb_image.h>
#include <string>

#include "Liara_Device.h"

namespace Liara::Graphics
{
    /**
     * @brief Main texture class for managing Vulkan textures.
     */
    class Liara_Texture
    {
    public:
        enum class TextureLoadResult : uint8_t
        {
            Success,
            FileNotFound,
            InvalidFormat,
            TooLarge,
            OutOfMemory,
            CorruptedData
        };

        /**
         * @brief Create texture from raw pixel data (most efficient)
         */
        static std::unique_ptr<Liara_Texture> CreateFromPixelData(Liara_Device& device,
                                                                  uint32_t width,
                                                                  uint32_t height,
                                                                  VkFormat format,
                                                                  std::span<const std::byte> pixelData,
                                                                  const Core::Liara_SettingsManager& settingsManager);

        /**
         * @brief Create texture from file (convenience method)
         */
        static std::unique_ptr<Liara_Texture> CreateFromFile(Liara_Device& device,
                                                             std::string_view filename,
                                                             const Core::Liara_SettingsManager& settingsManager);

        ~Liara_Texture();  ///< Destructor to clean up the texture
        Liara_Texture(const Liara_Texture&) = delete;
        Liara_Texture& operator=(const Liara_Texture&) = delete;

        [[nodiscard]] VkFormat GetFormat() const { return m_Format; }   ///< Get the format of the texture
        [[nodiscard]] VkDescriptorImageInfo GetDescriptorInfo() const;  ///< Get the descriptor info of the texture

    private:
        Liara_Texture(Liara_Device& device,
                      uint32_t width,
                      uint32_t height,
                      VkFormat format,
                      std::span<const std::byte> pixelData,
                      const Core::Liara_SettingsManager& settingsManager);

        void CreateTextureImage(std::span<const std::byte> pixelData);
        void CreateImage(VkMemoryPropertyFlags properties, VkImageUsageFlags usage);
        void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) const;
        void CopyBufferToImage(VkBuffer buffer, VkImage image) const;
        void CreateTextureImageView();
        void CreateTextureSampler();
        void GenerateMipmaps() const;

        Liara_Device& m_Device;
        const Core::Liara_SettingsManager& m_SettingsManager;

        VkImage m_Image{};
        VkDeviceMemory m_ImageMemory{};
        VkImageView m_ImageView{};
        VkSampler m_Sampler{};
        uint16_t m_MipLevels = 1;

        uint32_t m_Width;
        uint32_t m_Height;
        VkFormat m_Format;

        struct Builder
        {
            int width{}, height{}, channels{};
            VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
            std::unique_ptr<stbi_uc[], void (*)(void*)> pixels{nullptr, stbi_image_free};
            bool errorFlag = false;

            [[nodiscard]] TextureLoadResult LoadTexture(const std::string& filename,
                                                        const Core::Liara_SettingsManager& settingsManager);

            [[nodiscard]] std::span<const std::byte> GetPixelData() const noexcept {
                if (!pixels || errorFlag || width <= 0 || height <= 0) { return {}; }

                const size_t dataSize = static_cast<size_t>(width) * height * STBI_rgb_alpha;
                return std::span<const std::byte>{reinterpret_cast<const std::byte*>(pixels.get()), dataSize};
            }

            [[nodiscard]] bool IsValid() const noexcept { return !errorFlag && pixels && width > 0 && height > 0; }
        };
    };
}
