#pragma once

#include "Graphics/Liara_Device.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace Liara::Graphics::Descriptors
{
    class Liara_DescriptorAllocator
    {
    public:
        class Builder
        {
        public:
            explicit Builder(Liara_Device& device) : m_Device{device} {}
            Builder& AddPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& SetPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& SetMaxSets(uint32_t count);
            [[nodiscard]] std::unique_ptr<Liara_DescriptorAllocator> Build() const;

        private:
            Liara_Device& m_Device;
            std::vector<VkDescriptorPoolSize> m_PoolSizes{};
            uint32_t m_MaxSets = 1000;
            VkDescriptorPoolCreateFlags m_PoolFlags = 0;
        };

        explicit Liara_DescriptorAllocator(Liara_Device& device, uint32_t maxSets, VkDescriptorPoolCreateFlags flags,
                                     const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~Liara_DescriptorAllocator();
        Liara_DescriptorAllocator(const Liara_DescriptorAllocator&) = delete;
        Liara_DescriptorAllocator& operator=(const Liara_DescriptorAllocator&) = delete;

        bool Allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout);

        void ResetPools();

    private:
        VkDescriptorPool GrabPool();

        Liara_Device& m_Device;
        std::vector<VkDescriptorPoolSize> m_PoolSizes;
        uint32_t m_MaxSets;
        VkDescriptorPoolCreateFlags m_Flags;
        VkDescriptorPool m_CurrentPool{VK_NULL_HANDLE};
        std::vector<VkDescriptorPool> m_FreePools;
        std::vector<VkDescriptorPool> m_UsedPools;

        friend class Liara_DescriptorBuilder;
    };

    class Liara_DescriptorLayoutCache
    {
    public:
        class Builder
        {
        public:
            explicit Builder(Liara_Device& device) : m_Device{device} {}
            [[nodiscard]] std::unique_ptr<Liara_DescriptorLayoutCache> Build() const;

        private:
            Liara_Device& m_Device;
        };

        explicit Liara_DescriptorLayoutCache(Liara_Device& device) : m_Device{device} {}
        ~Liara_DescriptorLayoutCache();
        Liara_DescriptorLayoutCache(const Liara_DescriptorLayoutCache&) = delete;
        Liara_DescriptorLayoutCache& operator=(const Liara_DescriptorLayoutCache&) = delete;

        VkDescriptorSetLayout CreateLayout(const VkDescriptorSetLayoutCreateInfo* info);

    private:
        struct LayoutInfo
        {
            std::vector<VkDescriptorSetLayoutBinding> bindings;
            bool operator==(const LayoutInfo& other) const;
            [[nodiscard]] size_t hash() const;
        };

        struct LayoutHash { size_t operator()(const LayoutInfo& k) const { return k.hash(); } };

        Liara_Device& m_Device;
        std::unordered_map<LayoutInfo, VkDescriptorSetLayout, LayoutHash> m_LayoutCache;

        friend class Liara_DescriptorBuilder;
    };

    class Liara_DescriptorBuilder
    {
    public:
        Liara_DescriptorBuilder(Liara_DescriptorLayoutCache& layoutCache, Liara_DescriptorAllocator& allocator);
        ~Liara_DescriptorBuilder() = default;
        Liara_DescriptorBuilder(const Liara_DescriptorBuilder&) = delete;
        Liara_DescriptorBuilder& operator=(const Liara_DescriptorBuilder&) = delete;

        Liara_DescriptorBuilder& BindBuffer(uint32_t binding, const VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);
        Liara_DescriptorBuilder& BindImage(uint32_t binding, const VkDescriptorImageInfo* imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);

        bool Build(VkDescriptorSet& set, VkDescriptorSetLayout& layout);
        void Overwrite(const VkDescriptorSet& set);

    private:
        std::vector<VkWriteDescriptorSet> m_Writes;
        std::vector<VkDescriptorSetLayoutBinding> m_Bindings;
        Liara_DescriptorLayoutCache& m_Cache;
        Liara_DescriptorAllocator& m_Alloc;
    };
}
