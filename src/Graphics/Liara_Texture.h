#pragma once

#include "Liara_Device.h"

#include <string>
#include <stb_image.h>

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
            stbi_uc* pixels;
            bool errorFlag = false;

            ~Builder();

            void LoadTexture(const std::string& filename);
        };

        Liara_Texture(Liara_Device& device, const Builder& builder);
        ~Liara_Texture();

        Liara_Texture(const Liara_Texture&) = delete;
        Liara_Texture& operator=(const Liara_Texture&) = delete;

        [[nodiscard]] VkDescriptorImageInfo GetDescriptorInfo() const;

    private:
        void CreateTextureImage(const stbi_uc* pixels, uint32_t width, uint32_t height);
        void CreateImage(uint32_t width, uint32_t height, VkMemoryPropertyFlags properties);
        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const;
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;
        void CreateTextureImageView();
        void CreateTextureSampler();
        void GenerateMipmaps(int32_t texWidth, int32_t texHeight) const;

        Liara_Device& m_Device;

        VkImage m_Image{};
        VkDeviceMemory m_ImageMemory{};
        VkImageView m_ImageView{};
        VkSampler m_Sampler{};
        uint16_t m_MipLevels = 1;
    };
}