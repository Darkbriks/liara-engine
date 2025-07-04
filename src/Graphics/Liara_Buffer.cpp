#include "Liara_Buffer.h"

#include "Graphics/Liara_Device.h"

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <stdexcept>

#include "glm/fwd.hpp"
#include "glm/vec3.hpp"

namespace Liara::Graphics
{
    template <GpuCompatible T>
    Liara_Buffer::Liara_Buffer(Liara_Device& device, std::span<const T> data, const BufferConfig& config)
        : m_Device(device)
        , m_InstanceCount(static_cast<uint32_t>(data.size()))
        , m_InstanceSize(sizeof(T))
        , m_UsageFlags(config.usage)
        , m_MemoryPropertyFlags(config.memoryProperties)
        , m_MinOffsetAlignment(config.minOffsetAlignment) {
        if (data.empty()) { throw std::invalid_argument("Data span cannot be empty"); }

        m_AlignmentSize = GetAlignment(m_InstanceSize, m_MinOffsetAlignment);
        m_BufferSize = m_AlignmentSize * m_InstanceCount;

        CreateBuffer();

        if (m_MemoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            auto mapping = CreateMappingGuard();
            WriteData(data);
        }
        else {
            Liara_Buffer stagingBuffer(device, data, BufferConfig::Staging());
            auto stagingMapping = stagingBuffer.CreateMappingGuard();
            stagingBuffer.WriteData(data);
            stagingMapping.~MappingGuard();  // Explicit unmap

            device.CopyBuffer(stagingBuffer.GetBuffer(), m_Buffer, data.size_bytes());
        }
    }

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

    // Legacy constructor
    Liara_Buffer::Liara_Buffer(Liara_Device& device,
                               const VkDeviceSize instanceSize,
                               const uint32_t instanceCount,
                               const VkBufferUsageFlags usageFlags,
                               const VkMemoryPropertyFlags memoryPropertyFlags,
                               const VkDeviceSize minOffsetAlignment)
        : m_Device(device)
        , m_InstanceCount(instanceCount)
        , m_InstanceSize(instanceSize)
        , m_AlignmentSize(GetAlignment(instanceSize, minOffsetAlignment))
        , m_UsageFlags(usageFlags)
        , m_MemoryPropertyFlags(memoryPropertyFlags)
        , m_MinOffsetAlignment(minOffsetAlignment) {
        m_BufferSize = m_AlignmentSize * instanceCount;
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

    template <GpuCompatible T> void Liara_Buffer::WriteData(std::span<const T> data, const VkDeviceSize byteOffset) {
        ValidateAccess<T>(byteOffset, data.size());
        assert(m_Mapped && "Buffer must be mapped before writing");

        const VkDeviceSize dataSize = data.size_bytes();
        if (byteOffset + dataSize > m_BufferSize) { throw std::out_of_range("Write would exceed buffer bounds"); }

        auto* dst = static_cast<std::byte*>(m_Mapped) + byteOffset;
        std::memcpy(dst, data.data(), dataSize);
    }

    template <GpuCompatible T> void Liara_Buffer::WriteObject(const T& object, VkDeviceSize byteOffset) {
        WriteData(std::span<const T>{&object, 1}, byteOffset);
    }

    template <ContiguousRange Range>
    void Liara_Buffer::WriteRange(const Range& range, VkDeviceSize byteOffset)
        requires GpuCompatible<std::ranges::range_value_t<Range>>
    {
        using ValueType = std::ranges::range_value_t<Range>;
        std::span<const ValueType> dataSpan{std::ranges::data(range), std::ranges::size(range)};
        WriteData(dataSpan, byteOffset);
    }

    void Liara_Buffer::WriteBytes(const std::span<const std::byte> data, const VkDeviceSize byteOffset) const {
        assert(m_Mapped && "Buffer must be mapped before writing");

        if (byteOffset + data.size() > m_BufferSize) { throw std::out_of_range("Write would exceed buffer bounds"); }

        auto* dst = static_cast<std::byte*>(m_Mapped) + byteOffset;
        std::memcpy(dst, data.data(), data.size());
    }

    template <GpuCompatible T> std::span<T> Liara_Buffer::ReadData(size_t count, const VkDeviceSize byteOffset) {
        ValidateAccess<T>(byteOffset, count);
        assert(m_Mapped && "Buffer must be mapped before reading");

        auto* src = static_cast<T*>(static_cast<std::byte*>(m_Mapped) + byteOffset);
        return std::span<T>{src, count};
    }

    template <GpuCompatible T>
    std::span<const T> Liara_Buffer::ReadData(size_t count, const VkDeviceSize byteOffset) const {
        ValidateAccess<T>(byteOffset, count);
        assert(m_Mapped && "Buffer must be mapped before reading");

        const auto* src = static_cast<const T*>(static_cast<const std::byte*>(m_Mapped) + byteOffset);
        return std::span<const T>{src, count};
    }

    template <GpuCompatible T> T& Liara_Buffer::ReadObject(const VkDeviceSize byteOffset) {
        return ReadData<T>(1, byteOffset)[0];
    }

    template <GpuCompatible T> const T& Liara_Buffer::ReadObject(const VkDeviceSize byteOffset) const {
        return ReadData<T>(1, byteOffset)[0];
    }

    template <GpuCompatible T> void Liara_Buffer::WriteToIndex(const T& data, const uint32_t index) {
        if (index >= m_InstanceCount) { throw std::out_of_range("Index out of bounds"); }
        WriteObject(data, index * m_AlignmentSize);
    }

    template <GpuCompatible T> T& Liara_Buffer::ReadFromIndex(const uint32_t index) {
        if (index >= m_InstanceCount) { throw std::out_of_range("Index out of bounds"); }
        return ReadObject<T>(index * m_AlignmentSize);
    }

    template <GpuCompatible T> const T& Liara_Buffer::ReadFromIndex(const uint32_t index) const {
        if (index >= m_InstanceCount) { throw std::out_of_range("Index out of bounds"); }
        return ReadObject<T>(index * m_AlignmentSize);
    }

    template <typename T> void Liara_Buffer::ValidateAccess(const VkDeviceSize byteOffset, const size_t count) const {
        static_assert(GpuCompatible<T>, "Type must be GPU compatible");

        if (const VkDeviceSize requiredSize = byteOffset + (sizeof(T) * count); requiredSize > m_BufferSize) {
            throw std::out_of_range("Access would exceed buffer bounds");
        }

        if (byteOffset % alignof(T) != 0) {
            throw std::invalid_argument("Byte offset must be aligned to type alignment");
        }
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

    void Liara_Buffer::WriteToBuffer(const void* data, const VkDeviceSize size, const VkDeviceSize offset) const {
        assert(m_Mapped && "Cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE) { std::memcpy(m_Mapped, data, m_BufferSize); }
        else {
            auto* memOffset = static_cast<char*>(m_Mapped);
            memOffset += offset;
            std::memcpy(memOffset, data, size);
        }
    }

    void Liara_Buffer::WriteToIndex(const void* data, const int index) const {
        WriteToBuffer(data, m_InstanceSize, index * m_AlignmentSize);
    }

    template void Liara_Buffer::WriteData<float>(std::span<const float>, VkDeviceSize);
    template void Liara_Buffer::WriteData<uint32_t>(std::span<const uint32_t>, VkDeviceSize);
    template void Liara_Buffer::WriteData<glm::vec3>(std::span<const glm::vec3>, VkDeviceSize);
}