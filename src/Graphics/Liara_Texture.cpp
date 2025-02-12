#include "Liara_Texture.h"
#include "Liara_Buffer.h"

#include <cassert>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb/stb_image.h"

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

namespace Liara::Graphics
{
    Liara_Texture::Builder::~Builder()
    {
        stbi_image_free(pixels);
    }

    void Liara_Texture::Builder::LoadTexture(const std::string& filename, const int desiredChannels)
    {
        pixels = stbi_load((std::string(ENGINE_DIR) + filename).c_str(), &width, &height, &channels, desiredChannels);
        assert(pixels && "Failed to load texture image!");

        this->desiredChannels = desiredChannels;
    }

    Liara_Texture::Liara_Texture(Liara_Device& device, const Builder& builder) : m_Device(device)
    {
        CreateTextureImage(builder.pixels, builder.width, builder.height, builder.desiredChannels);
        CreateTextureImageView();
        CreateTextureSampler();
    }

    Liara_Texture::~Liara_Texture()
    {
        vkDestroySampler(m_Device.GetDevice(), m_Sampler, nullptr);
        vkDestroyImageView(m_Device.GetDevice(), m_ImageView, nullptr);
        vkDestroyImage(m_Device.GetDevice(), m_Image, nullptr);
        vkFreeMemory(m_Device.GetDevice(), m_ImageMemory, nullptr);
    }

    VkDescriptorImageInfo Liara_Texture::GetDescriptorInfo() const
    {
        return VkDescriptorImageInfo{
            .sampler = m_Sampler,
            .imageView = m_ImageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
    }


    void Liara_Texture::CreateTextureImage(const stbi_uc* pixels, const uint32_t width, const uint32_t height, const uint8_t channels)
    {
        assert(pixels && "No pixels data to create texture image");
        assert(width > 0 && height > 0 && "Invalid texture size");
        assert(channels > 0 && "Invalid number of channels");

        const VkDeviceSize imageSize = width * height * channels;

        Liara_Buffer stagingBuffer(
            m_Device,
            imageSize,
            1,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer(pixels);

        CreateImage(width, height, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        TransitionImageLayout(m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferToImage(stagingBuffer.GetBuffer(), m_Image, width, height);
        TransitionImageLayout(m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void Liara_Texture::CreateImage(const uint32_t width, const uint32_t height, const VkMemoryPropertyFlags properties)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(m_Device.GetDevice(), &imageInfo, nullptr, &m_Image) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_Device.GetDevice(), m_Image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = m_Device.FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(m_Device.GetDevice(), &allocInfo, nullptr, &m_ImageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate image memory");
        }

        if (vkBindImageMemory(m_Device.GetDevice(), m_Image, m_ImageMemory, 0) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to bind image memory.");
        }

    }

    void Liara_Texture::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const
    {
        VkCommandBuffer command_buffer = m_Device.BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            throw std::invalid_argument("Unsupported layout transition!");
        }


        vkCmdPipelineBarrier(
            command_buffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
            );

        m_Device.EndSingleTimeCommands(command_buffer);
    }

    void Liara_Texture::CopyBufferToImage(VkBuffer buffer, VkImage image, const uint32_t width, const uint32_t height) const
    {
        VkCommandBuffer command_buffer = m_Device.BeginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};

        vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        m_Device.EndSingleTimeCommands(command_buffer);
    }

    void Liara_Texture::CreateTextureImageView()
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_Image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB; // TODO : Adapt to the texture format
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_Device.GetDevice(), &viewInfo, nullptr, &m_ImageView) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create texture image view!");
        }
    }

    void Liara_Texture::CreateTextureSampler()
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR; // Or VK_FILTER_NEAREST, see https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler#page_Samplers
        samplerInfo.minFilter = VK_FILTER_LINEAR; // Or VK_FILTER_NEAREST
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // See https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler#page_Samplers
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        // TODO : Pass the anisotropy value as a parameter of the builder or the engine
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(m_Device.GetPhysicalDevice(), &properties);
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // Useful when using clamp to border addressing mode
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(m_Device.GetDevice(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create texture sampler!");
        }
    }
}