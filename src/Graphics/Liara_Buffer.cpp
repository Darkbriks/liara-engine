#include "Liara_Buffer.h"

#include "Graphics/Liara_Device.h"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <utility>


namespace Liara::Graphics
{

    Liara_Buffer::Liara_Buffer(Liara_Device& device, const VkDeviceSize size, const BufferConfig& config)
        : m_Device(device)
        , m_InstanceSize(size)
        , m_UsageFlags(config.usage)
        , m_MemoryPropertyFlags(config.memoryProperties)
        , m_MinOffsetAlignment(config.minOffsetAlignment) {
        if (size == 0) { throw std::invalid_argument("Buffer size cannot be zero"); }

        m_AlignmentSize = GetAlignment(m_InstanceSize, m_MinOffsetAlignment);
        m_BufferSize = m_AlignmentSize * m_InstanceCount;

        CreateBuffer();
    }

    Liara_Buffer::Liara_Buffer(Liara_Buffer&& other) noexcept
        : m_Device(other.m_Device)
        , m_Mapped(std::exchange(other.m_Mapped, nullptr))
        , m_Buffer(std::exchange(other.m_Buffer, VK_NULL_HANDLE))
        , m_Memory(std::exchange(other.m_Memory, VK_NULL_HANDLE))
        , m_BufferSize(other.m_BufferSize)
        , m_InstanceCount(other.m_InstanceCount)
        , m_InstanceSize(other.m_InstanceSize)
        , m_AlignmentSize(other.m_AlignmentSize)
        , m_UsageFlags(other.m_UsageFlags)
        , m_MemoryPropertyFlags(other.m_MemoryPropertyFlags)
        , m_MinOffsetAlignment(other.m_MinOffsetAlignment) {}

    Liara_Buffer& Liara_Buffer::operator=(Liara_Buffer&& other) noexcept {
        if (this != &other) {
            // Cleanup current resources
            Unmap();
            if (m_Buffer != VK_NULL_HANDLE) { vkDestroyBuffer(m_Device.GetDevice(), m_Buffer, nullptr); }
            if (m_Memory != VK_NULL_HANDLE) { vkFreeMemory(m_Device.GetDevice(), m_Memory, nullptr); }

            // Move from other
            m_Mapped = std::exchange(other.m_Mapped, nullptr);
            m_Buffer = std::exchange(other.m_Buffer, VK_NULL_HANDLE);
            m_Memory = std::exchange(other.m_Memory, VK_NULL_HANDLE);
            m_BufferSize = other.m_BufferSize;
            m_InstanceCount = other.m_InstanceCount;
            m_InstanceSize = other.m_InstanceSize;
            m_AlignmentSize = other.m_AlignmentSize;
            m_UsageFlags = other.m_UsageFlags;
            m_MemoryPropertyFlags = other.m_MemoryPropertyFlags;
            m_MinOffsetAlignment = other.m_MinOffsetAlignment;
        }
        return *this;
    }

    Liara_Buffer::~Liara_Buffer() {
        Unmap();
        if (m_Buffer != VK_NULL_HANDLE) { vkDestroyBuffer(m_Device.GetDevice(), m_Buffer, nullptr); }
        if (m_Memory != VK_NULL_HANDLE) { vkFreeMemory(m_Device.GetDevice(), m_Memory, nullptr); }
    }

    void Liara_Buffer::WriteBytes(const std::span<const std::byte> data, const VkDeviceSize byteOffset) const {
        assert(m_Mapped && "Buffer must be mapped before writing");

        if (byteOffset + data.size() > m_BufferSize) { throw std::out_of_range("Write would exceed buffer bounds"); }

        auto* dst = static_cast<std::byte*>(m_Mapped) + byteOffset;
        std::memcpy(dst, data.data(), data.size());
    }

    void Liara_Buffer::CreateBuffer() {
        m_Device.CreateBuffer(m_BufferSize, m_UsageFlags, m_MemoryPropertyFlags, m_Buffer, m_Memory);
    }

    VkDeviceSize Liara_Buffer::GetAlignment(const VkDeviceSize instanceSize, const VkDeviceSize minOffsetAlignment) {
        if (minOffsetAlignment > 0) { return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1); }
        return instanceSize;
    }

    VkResult Liara_Buffer::Map(const VkDeviceSize size, const VkDeviceSize offset) {
        assert(m_Buffer && m_Memory && "Called map on buffer before create");
        return vkMapMemory(m_Device.GetDevice(), m_Memory, offset, size, 0, &m_Mapped);
    }

    void Liara_Buffer::Unmap() {
        if (m_Mapped != nullptr) {
            vkUnmapMemory(m_Device.GetDevice(), m_Memory);
            m_Mapped = nullptr;
        }
    }

    VkResult Liara_Buffer::Flush(const VkDeviceSize size, const VkDeviceSize offset) const {
        const VkMappedMemoryRange mappedRange{.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                                              .pNext = nullptr,
                                              .memory = m_Memory,
                                              .offset = offset,
                                              .size = size};
        return vkFlushMappedMemoryRanges(m_Device.GetDevice(), 1, &mappedRange);
    }

    VkResult Liara_Buffer::Invalidate(const VkDeviceSize size, const VkDeviceSize offset) const {
        const VkMappedMemoryRange mappedRange{.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                                              .pNext = nullptr,
                                              .memory = m_Memory,
                                              .offset = offset,
                                              .size = size};
        return vkInvalidateMappedMemoryRanges(m_Device.GetDevice(), 1, &mappedRange);
    }

    VkResult Liara_Buffer::FlushIndex(const uint32_t index) const {
        return Flush(m_AlignmentSize, index * m_AlignmentSize);
    }

    VkResult Liara_Buffer::InvalidateIndex(const uint32_t index) const {
        return Invalidate(m_AlignmentSize, index * m_AlignmentSize);
    }

    VkDescriptorBufferInfo Liara_Buffer::DescriptorInfo(const VkDeviceSize size, const VkDeviceSize offset) const {
        return VkDescriptorBufferInfo{.buffer = m_Buffer, .offset = offset, .range = size};
    }

    VkDescriptorBufferInfo Liara_Buffer::DescriptorInfoForIndex(const uint32_t index) const {
        return DescriptorInfo(m_AlignmentSize, index * m_AlignmentSize);
    }

    template void Liara_Buffer::WriteData<float>(std::span<const float>, VkDeviceSize);
    template void Liara_Buffer::WriteData<uint32_t>(std::span<const uint32_t>, VkDeviceSize);
    template void Liara_Buffer::WriteData<glm::vec3>(std::span<const glm::vec3>, VkDeviceSize);
}