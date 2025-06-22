#pragma once

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
        /**
         * @brief Builder structure for loading a texture image.
         */
        struct Builder
        {
            int width{};                                ///< The width of the texture
            int height{};                               ///< The height of the texture
            int channels{};                             ///< The number of channels in the texture
            VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;  ///< The format of the texture
            stbi_uc* pixels{};                          ///< Pixels data of the texture
            /**
             * @brief Flag to check if there was an error while loading the texture
             * @note Can be triggered by a failed load or if the texture is too large. A message will be printed to
             * stderr
             */
            bool errorFlag = false;

            /**
             * @brief Destructor to free the pixel data
             */
            ~Builder();

            /**
             * @brief Loads a texture image from a file
             *
             * @param filename The file name of the texture to load
             * @param settingsManager The settings manager to use for texture settings
             */
            void LoadTexture(const std::string& filename, const Core::Liara_SettingsManager& settingsManager);
        };

        /**
         * @brief Main constructor for the Liara_Texture class
         * It's main purpose is to load a texture from a file
         *
         * @param device The device to create the texture on
         * @param builder The builder object containing the texture data
         * @param settingsManager The settings manager to use for texture settings
         */
        Liara_Texture(Liara_Device& device, const Builder& builder, const Core::Liara_SettingsManager& settingsManager);

        /**
         * @brief Constructor for the Liara_Texture class
         * It's main purpose is to create specific textures, like depth buffers or color attachments.
         *
         * @note Texture created with this constructor will not have any data loaded into them, and don't use mipmaps
         *
         * @warning Texture created with this constructor will not have a sampler
         *
         * @param device The device to create the texture on
         * @param width The width of the texture
         * @param height The height of the texture
         * @param format The format of the texture
         * @param usage The usage of the texture
         * @param settingsManager The settings manager to use for texture settings
         */
        Liara_Texture(Liara_Device& device,
                      int width,
                      int height,
                      VkFormat format,
                      VkImageUsageFlags usage,
                      const Core::Liara_SettingsManager& settingsManager);
        ~Liara_Texture();  ///< Destructor to clean up the texture

        Liara_Texture(const Liara_Texture&) = delete;
        Liara_Texture& operator=(const Liara_Texture&) = delete;

        [[nodiscard]] VkFormat GetFormat() const { return m_Format; }   ///< Get the format of the texture
        [[nodiscard]] VkDescriptorImageInfo GetDescriptorInfo() const;  ///< Get the descriptor info of the texture

    private:
        void CreateTextureImage(const stbi_uc* pixels);
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
    };
}
