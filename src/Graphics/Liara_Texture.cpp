#include "Liara_Texture.h"

#include "Core/Liara_SettingsManager.h"
#include "Graphics/Liara_Device.h"

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "Liara_Buffer.h"

#define STB_IMAGE_IMPLEMENTATION

#include <filesystem>
#include <stb/stb_image.h>

#ifndef ENGINE_DIR
    #define ENGINE_DIR "./"
#endif

namespace Liara::Graphics
{
    std::unique_ptr<Liara_Texture>
    Liara_Texture::CreateFromPixelData(Liara_Device& device,
                                       const uint32_t width,
                                       const uint32_t height,
                                       const VkFormat format,
                                       const std::span<const std::byte> pixelData,
                                       const Core::Liara_SettingsManager& settingsManager) {
        return std::unique_ptr<Liara_Texture>(
            new Liara_Texture(device, width, height, format, pixelData, settingsManager));
    }

    std::unique_ptr<Liara_Texture> Liara_Texture::CreateFromFile(Liara_Device& device,
                                                                 const std::string_view filename,
                                                                 const Core::Liara_SettingsManager& settingsManager) {
        Builder builder;
        if (const auto result = builder.LoadTexture(std::string(filename), settingsManager);
            result != TextureLoadResult::Success) {
            switch (result) {
                case TextureLoadResult::FileNotFound:
                    LIARA_THROW_RUNTIME_ERROR(LogRendering, "Texture file not found: {}", filename);
                case TextureLoadResult::InvalidFormat:
                    LIARA_THROW_RUNTIME_ERROR(LogRendering, "Invalid texture format for file: {}", filename);
                case TextureLoadResult::TooLarge:
                    LIARA_THROW_RUNTIME_ERROR(LogRendering, "Texture file too large: {}", filename);
                case TextureLoadResult::OutOfMemory:
                    LIARA_THROW_RUNTIME_ERROR(LogRendering, "Out of memory while loading texture: {}", filename);
                case TextureLoadResult::CorruptedData:
                    LIARA_THROW_RUNTIME_ERROR(LogRendering, "Corrupted texture data in file: {}", filename);
                default: LIARA_THROW_RUNTIME_ERROR(LogRendering, "Unknown error loading texture: {}", filename);
            }
        }

        LIARA_CHECK_RUNTIME(builder.IsValid(), LogRendering, "Failed to load texture: {}", filename);

        return CreateFromPixelData(device,
                                   static_cast<uint32_t>(builder.width),
                                   static_cast<uint32_t>(builder.height),
                                   builder.format,
                                   builder.GetPixelData(),
                                   settingsManager);
    }

    Liara_Texture::Liara_Texture(Liara_Device& device,
                                 const uint32_t width,
                                 const uint32_t height,
                                 const VkFormat format,
                                 const std::span<const std::byte> pixelData,
                                 const Core::Liara_SettingsManager& settingsManager)
        : m_Device(device)
        , m_SettingsManager(settingsManager)
        , m_Width(width)
        , m_Height(height)
        , m_Format(format) {
        LIARA_CHECK_RUNTIME(
            !pixelData.empty() && width != 0 && height != 0, LogRendering, "Invalid texture parameters");

        m_MipLevels = settingsManager.GetBool("texture.use_mipmaps")
                          ? static_cast<uint16_t>(std::floor(std::log2(std::max(width, height)))) + 1
                          : 1;

        CreateTextureImage(pixelData);
        CreateTextureImageView();
        CreateTextureSampler();
    }

    Liara_Texture::~Liara_Texture() {
        vkDestroySampler(m_Device.GetDevice(), m_Sampler, nullptr);
        vkDestroyImageView(m_Device.GetDevice(), m_ImageView, nullptr);
        vkDestroyImage(m_Device.GetDevice(), m_Image, nullptr);
        vkFreeMemory(m_Device.GetDevice(), m_ImageMemory, nullptr);
    }

    VkDescriptorImageInfo Liara_Texture::GetDescriptorInfo() const {
        return VkDescriptorImageInfo{
            .sampler = m_Sampler, .imageView = m_ImageView, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    void Liara_Texture::CreateTextureImage(std::span<const std::byte> pixelData) {
        assert(!pixelData.empty() && "No pixel data to create texture image");
        assert(m_Width > 0 && m_Height > 0 && "Invalid texture size");

        LIARA_CHECK_ARGUMENT(pixelData.size_bytes() == static_cast<VkDeviceSize>(m_Width) * m_Height * STBI_rgb_alpha,
                             LogRendering,
                             "Pixel data size mismatch (expected {}, got {})",
                             static_cast<VkDeviceSize>(m_Width) * m_Height * STBI_rgb_alpha,
                             pixelData.size_bytes());

        const auto stagingBuffer = std::make_unique<Liara_Buffer>(m_Device, pixelData, BufferConfig::Staging());

        CreateImage(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

        TransitionImageLayout(m_Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferToImage(stagingBuffer->GetBuffer(), m_Image);
        GenerateMipmaps();
    }

    void Liara_Texture::CreateImage(const VkMemoryPropertyFlags properties, VkImageUsageFlags usage) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_Width;
        imageInfo.extent.height = m_Height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = m_MipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = m_Format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        m_Device.CreateImageWithInfo(imageInfo, properties, m_Image, m_ImageMemory);
    }

    void Liara_Texture::TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) const {
        VkCommandBuffer commandBuffer = m_Device.BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = m_MipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage = 0;
        VkPipelineStageFlags destinationStage = 0;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
                 && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else { LIARA_THROW_RUNTIME_ERROR(LogRendering, "Unsupported layout transition!"); }


        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        m_Device.EndSingleTimeCommands(commandBuffer);
    }

    void Liara_Texture::CopyBufferToImage(VkBuffer buffer, VkImage image) const {
        VkCommandBuffer commandBuffer = m_Device.BeginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {.x = 0, .y = 0, .z = 0};
        region.imageExtent = {.width = m_Width, .height = m_Height, .depth = 1};

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        m_Device.EndSingleTimeCommands(commandBuffer);
    }

    void Liara_Texture::CreateTextureImageView() {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_Image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_Format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = m_MipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_Device.GetDevice(), &viewInfo, nullptr, &m_ImageView) != VK_SUCCESS) {
            LIARA_THROW_RUNTIME_ERROR(LogRendering, "Failed to create texture image view!");
        }
    }

    void Liara_Texture::CreateTextureSampler() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter =
            VK_FILTER_LINEAR;  // Or VK_FILTER_NEAREST, see
                               // https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler#page_Samplers
        samplerInfo.minFilter = VK_FILTER_LINEAR;  // Or VK_FILTER_NEAREST
        samplerInfo.addressModeU =
            VK_SAMPLER_ADDRESS_MODE_REPEAT;  // See
                                             // https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler#page_Samplers
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        samplerInfo.anisotropyEnable =
            m_SettingsManager.GetBool("texture.use_anisotropic_filtering") ? VK_TRUE : VK_FALSE;
        samplerInfo.maxAnisotropy = static_cast<float>(m_SettingsManager.GetUInt("texture.max_anisotropy"));

        samplerInfo.borderColor =
            VK_BORDER_COLOR_INT_OPAQUE_BLACK;  // Useful when using clamp to border addressing mode
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(m_MipLevels);

        if (vkCreateSampler(m_Device.GetDevice(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {
            LIARA_THROW_RUNTIME_ERROR(LogRendering, "Failed to create texture sampler!");
        }
    }

    // https://vulkan-tutorial.com/Generating_Mipmaps
    // Generate mipmaps at runtime is not recommended.
    // It is better to generate them offline and load them directly.
    // So, it can be useful to create a tool to generate mipmaps offline in the future.
    void Liara_Texture::GenerateMipmaps() const {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(m_Device.GetPhysicalDevice(), m_Format, &formatProperties);

        if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == 0u) {
            LIARA_LOG_WARNING(LogRendering,
                              "Texture image format does not support linear blitting! Mipmaps will not be generated.");
            return;
        }

        VkCommandBuffer commandBuffer = m_Device.BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = m_Image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        auto mipWidth = static_cast<int32_t>(m_Width);
        auto mipHeight = static_cast<int32_t>(m_Height);

        for (uint16_t i = 1; i < m_MipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0] = {.x = 0, .y = 0, .z = 0};
            blit.srcOffsets[1] = {.x = mipWidth, .y = mipHeight, .z = 1};
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = {.x = 0, .y = 0, .z = 0};
            blit.dstOffsets[1] = {.x = mipWidth > 1 ? mipWidth / 2 : 1, .y = mipHeight > 1 ? mipHeight / 2 : 1, .z = 1};
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(commandBuffer,
                           m_Image,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           m_Image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &blit,
                           VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &barrier);

            if (mipWidth > 1) { mipWidth /= 2; }
            if (mipHeight > 1) { mipHeight /= 2; }
        }

        barrier.subresourceRange.baseMipLevel = m_MipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier);

        m_Device.EndSingleTimeCommands(commandBuffer);
    }

    Liara_Texture::TextureLoadResult
    Liara_Texture::Builder::LoadTexture(const std::string& filename,
                                        const Core::Liara_SettingsManager& settingsManager) {
        pixels.reset();
        errorFlag = false;
        width = height = channels = 0;

        const std::string fullPath = std::string(ENGINE_DIR) + filename;

        if (!std::filesystem::exists(fullPath)) {
            errorFlag = true;
            return TextureLoadResult::FileNotFound;
        }

        stbi_uc* rawPixels = stbi_load(fullPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

        if (!rawPixels) {
            errorFlag = true;
            LIARA_LOG_ERROR(LogRendering, "STBI load failed for {}: {}", filename, stbi_failure_reason());

            const std::string stbiError = stbi_failure_reason();
            if (stbiError.find("unsupported") != std::string::npos) { return TextureLoadResult::InvalidFormat; }
            if (stbiError.find("corrupt") != std::string::npos) { return TextureLoadResult::CorruptedData; }
            return TextureLoadResult::InvalidFormat;
        }

        pixels = std::unique_ptr<stbi_uc[], void (*)(void*)>{rawPixels, stbi_image_free};

        if (width <= 0 || height <= 0) {
            errorFlag = true;
            return TextureLoadResult::InvalidFormat;
        }

        if (const uint32_t maxDimension = std::max(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
            maxDimension > settingsManager.GetUInt("texture.max_size")) {
            errorFlag = true;
            return TextureLoadResult::TooLarge;
        }

        return TextureLoadResult::Success;
    }
}