#include "Liara_Descriptor.h"

#include <cassert>
#include <stdexcept>

namespace Liara::Graphics::Descriptors
{
    // *************** Descriptor Set Layout Builder *********************

    Liara_DescriptorSetLayout::Builder& Liara_DescriptorSetLayout::Builder::AddBinding(
        const uint32_t binding,
        const VkDescriptorType descriptorType,
        const VkShaderStageFlags stageFlags,
        const uint32_t count)
    {
        assert(m_Bindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        m_Bindings[binding] = layoutBinding;
        return *this;
    }

    std::unique_ptr<Liara_DescriptorSetLayout> Liara_DescriptorSetLayout::Builder::Build() const
    {
        return std::make_unique<Liara_DescriptorSetLayout>(m_Device, m_Bindings);
    }

    // *************** Descriptor Set Layout *********************

    Liara_DescriptorSetLayout::Liara_DescriptorSetLayout(
        Liara_Device& device, const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>& bindings)
        : m_Device(device), m_Bindings(bindings)
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        setLayoutBindings.reserve(bindings.size());
        for (auto [fst, snd]: bindings) { setLayoutBindings.push_back(snd); }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(
                device.GetDevice(),
                &descriptorSetLayoutInfo,
                nullptr,
                &m_DescriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    Liara_DescriptorSetLayout::~Liara_DescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(m_Device.GetDevice(), m_DescriptorSetLayout, nullptr);
    }

    // *************** Descriptor Pool Builder *********************

    Liara_DescriptorPool::Builder& Liara_DescriptorPool::Builder::AddPoolSize(
        const VkDescriptorType descriptorType, const uint32_t count)
    {
        m_PoolSizes.push_back({descriptorType, count});
        return *this;
    }

    Liara_DescriptorPool::Builder& Liara_DescriptorPool::Builder::SetPoolFlags(const VkDescriptorPoolCreateFlags flags)
    {
        m_PoolFlags = flags;
        return *this;
    }

    Liara_DescriptorPool::Builder& Liara_DescriptorPool::Builder::SetMaxSets(const uint32_t count)
    {
        m_MaxSets = count;
        return *this;
    }

    std::unique_ptr<Liara_DescriptorPool> Liara_DescriptorPool::Builder::Build() const
    {
        return std::make_unique<Liara_DescriptorPool>(m_Device, m_MaxSets, m_PoolFlags, m_PoolSizes);
    }

    // *************** Descriptor Pool *********************

    Liara_DescriptorPool::Liara_DescriptorPool(
        Liara_Device& device,
        const uint32_t maxSets,
        const VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize>& poolSizes)
        : m_Device(device)
    {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        if (vkCreateDescriptorPool(device.GetDevice(), &descriptorPoolInfo, nullptr, &m_DescriptorPool) !=
            VK_SUCCESS) { throw std::runtime_error("failed to create descriptor pool!"); }
    }

    Liara_DescriptorPool::~Liara_DescriptorPool()
    {
        vkDestroyDescriptorPool(m_Device.GetDevice(), m_DescriptorPool, nullptr);
    }

    bool Liara_DescriptorPool::AllocateDescriptor(
        VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        // TODO: See https://vkguide.dev/docs/extra-chapter/abstracting-descriptor/ to abstract this
        if (vkAllocateDescriptorSets(m_Device.GetDevice(), &allocInfo, &descriptor) != VK_SUCCESS) { return false; }
        return true;
    }

    void Liara_DescriptorPool::FreeDescriptors(const std::vector<VkDescriptorSet>& descriptors) const
    {
        vkFreeDescriptorSets(
            m_Device.GetDevice(),
            m_DescriptorPool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    void Liara_DescriptorPool::ResetPool() const { vkResetDescriptorPool(m_Device.GetDevice(), m_DescriptorPool, 0); }

    // *************** Descriptor Writer *********************

    Liara_DescriptorWriter::Liara_DescriptorWriter(Liara_DescriptorSetLayout& setLayout, Liara_DescriptorPool& pool)
        : m_SetLayout(setLayout), m_Pool(pool) {}

    Liara_DescriptorWriter& Liara_DescriptorWriter::WriteBuffer(
        const uint32_t binding, const VkDescriptorBufferInfo* bufferInfo)
    {
        assert(m_SetLayout.m_Bindings.count(binding) == 1 && "Layout does not contain specified binding");

        const auto& bindingDescription = m_SetLayout.m_Bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        m_Writes.push_back(write);
        return *this;
    }

    Liara_DescriptorWriter& Liara_DescriptorWriter::WriteImage(
        const uint32_t binding, const VkDescriptorImageInfo* imageInfo)
    {
        assert(m_SetLayout.m_Bindings.count(binding) == 1 && "Layout does not contain specified binding");

        const auto& bindingDescription = m_SetLayout.m_Bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        m_Writes.push_back(write);
        return *this;
    }

    bool Liara_DescriptorWriter::Build(VkDescriptorSet& set)
    {
        if (const bool success = m_Pool.AllocateDescriptor(m_SetLayout.GetDescriptorSetLayout(), set); !success)
        {
            return false;
        }
        Overwrite(set);
        return true;
    }

    void Liara_DescriptorWriter::Overwrite(const VkDescriptorSet& set)
    {
        for (auto& write: m_Writes) { write.dstSet = set; }
        vkUpdateDescriptorSets(m_Pool.m_Device.GetDevice(), m_Writes.size(), m_Writes.data(), 0, nullptr);
    }
}
