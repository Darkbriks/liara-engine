#pragma once

#include <vulkan/vulkan_core.h>

#include <concepts>
#include <ranges>
#include <span>
#include <type_traits>

#include "Liara_Device.h"

namespace Liara::Graphics
{
    template <typename T>
    concept TriviallyCopiable = std::is_trivially_copyable_v<T>;

    template <typename T>
    concept GpuCompatible = TriviallyCopiable<T> && std::is_standard_layout_v<T>;

    template <typename Range>
    concept ContiguousRange = std::ranges::contiguous_range<Range> && std::ranges::sized_range<Range>;

    struct BufferConfig
    {
        VkBufferUsageFlags usage;
        VkMemoryPropertyFlags memoryProperties;
        VkDeviceSize minOffsetAlignment = 1;

        // Predefined configurations for common buffer types
        static constexpr BufferConfig Vertex() {
            return {.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    .memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};
        }

        static constexpr BufferConfig Index() {
            return {.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    .memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};
        }

        static constexpr BufferConfig Uniform(const uint64_t minOffsetAlignment = 256) {
            return {.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    .memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    .minOffsetAlignment = minOffsetAlignment};
        }

        static constexpr BufferConfig Staging() {
            return {.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    .memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        }
    };

    class Liara_Buffer
    {
    public:
        /**
         * @brief Modern constructor with span and config
         */
        template <GpuCompatible T>
        Liara_Buffer(Liara_Device& device, std::span<const T> data, const BufferConfig& config);

        /**
         * @brief Constructor for empty buffer (e.g., dynamic UBOs)
         */
        Liara_Buffer(Liara_Device& device, VkDeviceSize size, const BufferConfig& config);

        ~Liara_Buffer();
        Liara_Buffer(const Liara_Buffer&) = delete;
        Liara_Buffer& operator=(const Liara_Buffer&) = delete;
        Liara_Buffer(Liara_Buffer&&) noexcept;
        Liara_Buffer& operator=(Liara_Buffer&&) noexcept;

        /**
         * @brief Write typed data span to buffer
         */
        template <GpuCompatible T> void WriteData(std::span<const T> data, VkDeviceSize byteOffset = 0);

        /**
         * @brief Write single object to buffer
         */
        template <GpuCompatible T> void WriteObject(const T& object, VkDeviceSize byteOffset = 0);

        /**
         * @brief Write range/container to buffer
         */
        template <ContiguousRange Range>
        void WriteRange(const Range& range, VkDeviceSize byteOffset = 0)
            requires GpuCompatible<std::ranges::range_value_t<Range>>;

        /**
         * @brief Write raw bytes (for compatibility with legacy code)
         */
        void WriteBytes(std::span<const std::byte> data, VkDeviceSize byteOffset = 0) const;

        /**
         * @brief Read typed data from mapped buffer
         */
        template <GpuCompatible T> [[nodiscard]] std::span<T> ReadData(size_t count, VkDeviceSize byteOffset = 0);

        template <GpuCompatible T>
        [[nodiscard]] std::span<const T> ReadData(size_t count, VkDeviceSize byteOffset = 0) const;

        /**
         * @brief Read single object from mapped buffer
         */
        template <GpuCompatible T> [[nodiscard]] T& ReadObject(VkDeviceSize byteOffset = 0);

        template <GpuCompatible T> [[nodiscard]] const T& ReadObject(VkDeviceSize byteOffset = 0) const;

        [[nodiscard]] VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void Unmap();
        [[nodiscard]] bool IsMapped() const noexcept { return m_Mapped != nullptr; }

        /**
         * @brief RAII mapping helper
         */
        class MappingGuard
        {
        public:
            explicit MappingGuard(Liara_Buffer& buffer,
                                  const VkDeviceSize size = VK_WHOLE_SIZE,
                                  const VkDeviceSize offset = 0)
                : m_Buffer(&buffer) {
                if (const auto result = m_Buffer->Map(size, offset); result != VK_SUCCESS) {
                    throw std::runtime_error("Failed to map buffer");
                }
            }

            ~MappingGuard() {
                if (m_Buffer) { m_Buffer->Unmap(); }
            }

            MappingGuard(const MappingGuard&) = delete;
            MappingGuard& operator=(const MappingGuard&) = delete;
            MappingGuard(MappingGuard&& other) noexcept
                : m_Buffer(std::exchange(other.m_Buffer, nullptr)) {}

            MappingGuard& operator=(MappingGuard&& other) noexcept {
                if (this != &other) {
                    if (m_Buffer) { m_Buffer->Unmap(); }
                    m_Buffer = std::exchange(other.m_Buffer, nullptr);
                }
                return *this;
            }

        private:
            Liara_Buffer* m_Buffer;
        };

        [[nodiscard]] MappingGuard CreateMappingGuard(const VkDeviceSize size = VK_WHOLE_SIZE,
                                                      const VkDeviceSize offset = 0) {
            return MappingGuard(*this, size, offset);
        }

        [[nodiscard]] VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
        [[nodiscard]] VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

        template <GpuCompatible T> void WriteToIndex(const T& data, uint32_t index);

        template <GpuCompatible T> [[nodiscard]] T& ReadFromIndex(uint32_t index);

        template <GpuCompatible T> [[nodiscard]] const T& ReadFromIndex(uint32_t index) const;

        [[nodiscard]] VkResult FlushIndex(uint32_t index) const;
        [[nodiscard]] VkResult InvalidateIndex(uint32_t index) const;

        [[nodiscard]] VkDescriptorBufferInfo DescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE,
                                                            VkDeviceSize offset = 0) const;
        [[nodiscard]] VkDescriptorBufferInfo DescriptorInfoForIndex(uint32_t index) const;

        [[nodiscard]] VkBuffer GetBuffer() const noexcept { return m_Buffer; }
        [[nodiscard]] void* GetMappedMemory() const noexcept { return m_Mapped; }
        [[nodiscard]] VkDeviceSize GetSize() const noexcept { return m_BufferSize; }
        [[nodiscard]] uint32_t GetInstanceCount() const noexcept { return m_InstanceCount; }
        [[nodiscard]] VkDeviceSize GetInstanceSize() const noexcept { return m_InstanceSize; }
        [[nodiscard]] VkDeviceSize GetAlignmentSize() const noexcept { return m_AlignmentSize; }
        [[nodiscard]] VkBufferUsageFlags GetUsageFlags() const noexcept { return m_UsageFlags; }
        [[nodiscard]] VkMemoryPropertyFlags GetMemoryPropertyFlags() const noexcept { return m_MemoryPropertyFlags; }

    private:
        void CreateBuffer();
        static VkDeviceSize GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

        template <typename T> void ValidateAccess(VkDeviceSize byteOffset, size_t count = 1) const;

        Liara_Device& m_Device;
        void* m_Mapped = nullptr;
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_Memory = VK_NULL_HANDLE;

        VkDeviceSize m_BufferSize;
        uint32_t m_InstanceCount = 1;
        VkDeviceSize m_InstanceSize;
        VkDeviceSize m_AlignmentSize;
        VkBufferUsageFlags m_UsageFlags;
        VkMemoryPropertyFlags m_MemoryPropertyFlags;
        VkDeviceSize m_MinOffsetAlignment;
    };

    /**
     * @brief Create buffer from data with automatic sizing
     */
    template <GpuCompatible T>
    [[nodiscard]] std::unique_ptr<Liara_Buffer>
    CreateBufferFromData(Liara_Device& device, std::span<const T> data, const BufferConfig& config) {
        return std::make_unique<Liara_Buffer>(device, data, config);
    }

    /**
     * @brief Create buffer from container
     */
    template <ContiguousRange Range>
    [[nodiscard]] std::unique_ptr<Liara_Buffer>
    CreateBufferFromRange(Liara_Device& device, const Range& range, const BufferConfig& config)
        requires GpuCompatible<std::ranges::range_value_t<Range>>
    {
        std::span<const std::ranges::range_value_t<Range>> dataSpan{std::ranges::data(range), std::ranges::size(range)};
        return CreateBufferFromData(device, dataSpan, config);
    }
}

#include "Liara_Buffer.tpp"  // IWYU pragma: keep