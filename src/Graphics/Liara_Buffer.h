#pragma once

#include "Liara_Device.h"

namespace Liara::Graphics
{
    class Liara_Buffer
    {
    public:
        Liara_Buffer(
            Liara_Device &device,
            VkDeviceSize instanceSize,
            uint32_t instanceCount,
            VkBufferUsageFlags usageFlags,
            VkMemoryPropertyFlags memoryPropertyFlags,
            VkDeviceSize minOffsetAlignment = 1);

        ~Liara_Buffer();
 
        Liara_Buffer(const Liara_Buffer&) = delete;
        Liara_Buffer& operator=(const Liara_Buffer&) = delete;
 
        VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void Unmap();
 
        void WriteToBuffer(const void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
        [[nodiscard]] VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
        [[nodiscard]] VkDescriptorBufferInfo DescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
        [[nodiscard]] VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
 
        void WriteToIndex(const void* data, int index) const;
        [[nodiscard]] VkResult FlushIndex(int index) const;
        [[nodiscard]] VkDescriptorBufferInfo DescriptorInfoForIndex(int index) const;
        [[nodiscard]] VkResult InvalidateIndex(int index) const;
 
        [[nodiscard]] VkBuffer GetBuffer() const { return m_Buffer; }
        [[nodiscard]] void* GetMappedMemory() const { return m_Mapped; }
        [[nodiscard]] uint32_t GetInstanceCount() const { return m_InstanceCount; }
        [[nodiscard]] VkDeviceSize GetInstanceSize() const { return m_InstanceSize; }
        [[nodiscard]] VkDeviceSize GetAlignmentSize() const { return m_InstanceSize; }
        [[nodiscard]] VkBufferUsageFlags GetUsageFlags() const { return m_UsageFlags; }
        [[nodiscard]] VkMemoryPropertyFlags GetMemoryPropertyFlags() const { return m_MemoryPropertyFlags; }
        [[nodiscard]] VkDeviceSize GetBufferSize() const { return m_BufferSize; }
 
    private:
        static VkDeviceSize GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);
 
        Liara_Device& m_Device;
        void* m_Mapped = nullptr;
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_Memory = VK_NULL_HANDLE;
 
        VkDeviceSize m_BufferSize;
        uint32_t m_InstanceCount;
        VkDeviceSize m_InstanceSize;
        VkDeviceSize m_AlignmentSize;
        VkBufferUsageFlags m_UsageFlags;
        VkMemoryPropertyFlags m_MemoryPropertyFlags;
    };
};