#pragma once

#include "Liara_Device.h"

#include <memory>
#include <string>

#include "../external/stb/stb_image.h"

namespace Liara::Graphics
{
    class Liara_Texture
    {
    public:
        struct Builder
        {
            int width;
            int height;
            int channels;
            int desiredChannels;
            stbi_uc* pixels;

            ~Builder();

            void LoadTexture(const std::string& filename, int desiredChannels = STBI_rgb_alpha);
        };

        Liara_Texture(Liara_Device& device, const Builder& builder);
        ~Liara_Texture();

        Liara_Texture(const Liara_Texture&) = delete;
        Liara_Texture& operator=(const Liara_Texture&) = delete;

        VkDescriptorImageInfo GetDescriptorInfo() const;

    private:
        void CreateTextureImage(const stbi_uc* pixels, uint32_t width, uint32_t height, uint8_t channels);
        void CreateImage(uint32_t width, uint32_t height, VkMemoryPropertyFlags properties);
        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const;
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;
        void CreateTextureImageView();
        void CreateTextureSampler();

        Liara_Device& m_Device;

        VkImage m_Image;
        VkDeviceMemory m_ImageMemory;
        VkImageView m_ImageView;
        VkSampler m_Sampler;
    };
}
