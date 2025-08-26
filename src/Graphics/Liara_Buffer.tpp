#pragma once

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
        LIARA_CHECK_ARGUMENT(!data.empty(), LogBuffer, "Data span cannot be empty");

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

    template <GpuCompatible T> void Liara_Buffer::WriteData(std::span<const T> data, const VkDeviceSize byteOffset) {
        ValidateAccess<T>(byteOffset, data.size());
        assert(m_Mapped && "Buffer must be mapped before writing");

        const VkDeviceSize dataSize = data.size_bytes();
        LIARA_CHECK_OUT_OF_RANGE(byteOffset + dataSize <= m_BufferSize, LogBuffer, "Write would exceed buffer bounds");

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
        LIARA_CHECK_OUT_OF_RANGE(index < m_InstanceCount, LogBuffer, "Index out of bounds");
        WriteObject(data, index * m_AlignmentSize);
    }

    template <GpuCompatible T> T& Liara_Buffer::ReadFromIndex(const uint32_t index) {
        LIARA_CHECK_OUT_OF_RANGE(index < m_InstanceCount, LogBuffer, "Index out of bounds");
        return ReadObject<T>(index * m_AlignmentSize);
    }

    template <GpuCompatible T> const T& Liara_Buffer::ReadFromIndex(const uint32_t index) const {
        LIARA_CHECK_OUT_OF_RANGE(index < m_InstanceCount, LogBuffer, "Index out of bounds");
        return ReadObject<T>(index * m_AlignmentSize);
    }

    template <typename T> void Liara_Buffer::ValidateAccess(const VkDeviceSize byteOffset, const size_t count) const {
        static_assert(GpuCompatible<T>, "Type must be GPU compatible");

        if (const VkDeviceSize requiredSize = byteOffset + (sizeof(T) * count); requiredSize > m_BufferSize) {
            LIARA_THROW_OUT_OF_RANGE(LogBuffer, "Access would exceed buffer bounds");
        }

        LIARA_CHECK_ARGUMENT(byteOffset % alignof(T) == 0, LogBuffer, "Byte offset must be aligned to type alignment");
    }
}