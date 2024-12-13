#pragma once

#include "Graphics/Liara_Device.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace Liara::Graphics::Descriptors
{
    class Liara_DescriptorSetLayout
    {
    public:
        class Builder
        {
        public:
            explicit Builder(Liara_Device& device) : m_Device{device} {}

            Builder& AddBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags,
                                uint32_t count = 1);

            std::unique_ptr<Liara_DescriptorSetLayout> Build() const;

        private:
            Liara_Device& m_Device;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_Bindings{};
        };

        Liara_DescriptorSetLayout(Liara_Device& device,
                                    const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>& bindings);
        ~Liara_DescriptorSetLayout();
        Liara_DescriptorSetLayout(const Liara_DescriptorSetLayout&) = delete;
        Liara_DescriptorSetLayout& operator=(const Liara_DescriptorSetLayout&) = delete;

        VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }

    private:
        Liara_Device& m_Device;
        VkDescriptorSetLayout m_DescriptorSetLayout{};
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_Bindings;

        friend class Liara_DescriptorWriter;
    };

    class Liara_DescriptorPool
    {
    public:
        class Builder
        {
        public:
            explicit Builder(Liara_Device& device) : m_Device{device} {}

            Builder& AddPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& SetPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& SetMaxSets(uint32_t count);

            [[nodiscard]] std::unique_ptr<Liara_DescriptorPool> Build() const;

        private:
            Liara_Device& m_Device;
            std::vector<VkDescriptorPoolSize> m_PoolSizes{};
            uint32_t m_MaxSets = 1000;
            VkDescriptorPoolCreateFlags m_PoolFlags = 0;
        };

        Liara_DescriptorPool(Liara_Device& device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
                          const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~Liara_DescriptorPool();
        Liara_DescriptorPool(const Liara_DescriptorPool&) = delete;
        Liara_DescriptorPool& operator=(const Liara_DescriptorPool&) = delete;

        bool AllocateDescriptor(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;
        void FreeDescriptors(const std::vector<VkDescriptorSet>& descriptors) const;
        void ResetPool() const;

    private:
        Liara_Device& m_Device;
        VkDescriptorPool m_DescriptorPool{};

        friend class Liara_DescriptorWriter;
    };

    class Liara_DescriptorWriter
    {
    public:
        Liara_DescriptorWriter(Liara_DescriptorSetLayout& setLayout, Liara_DescriptorPool& pool);

        Liara_DescriptorWriter& WriteBuffer(uint32_t binding, const VkDescriptorBufferInfo* bufferInfo);
        Liara_DescriptorWriter& WriteImage(uint32_t binding, const VkDescriptorImageInfo* imageInfo);

        bool Build(VkDescriptorSet& set);
        void Overwrite(const VkDescriptorSet& set);

    private:
        Liara_DescriptorSetLayout& m_SetLayout;
        Liara_DescriptorPool& m_Pool;
        std::vector<VkWriteDescriptorSet> m_Writes;
    };
}
