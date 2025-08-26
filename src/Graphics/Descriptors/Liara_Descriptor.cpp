#include "Liara_Descriptor.h"

#include "Graphics/Liara_Device.h"

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <vector>

namespace Liara::Graphics::Descriptors
{
    // *************** Descriptor Allocator Builder *********************

    Liara_DescriptorAllocator::Builder&
    Liara_DescriptorAllocator::Builder::AddPoolSize(const VkDescriptorType descriptorType, const uint32_t count) {
        m_PoolSizes.push_back({descriptorType, count});
        return *this;
    }

    Liara_DescriptorAllocator::Builder&
    Liara_DescriptorAllocator::Builder::SetPoolFlags(const VkDescriptorPoolCreateFlags flags) {
        m_PoolFlags = flags;
        return *this;
    }

    Liara_DescriptorAllocator::Builder& Liara_DescriptorAllocator::Builder::SetMaxSets(const uint32_t count) {
        m_MaxSets = count;
        return *this;
    }

    std::unique_ptr<Liara_DescriptorAllocator> Liara_DescriptorAllocator::Builder::Build() const {
        return std::make_unique<Liara_DescriptorAllocator>(m_Device, m_MaxSets, m_PoolFlags, m_PoolSizes);
    }

    // *************** Descriptor Allocator Implementation *********************

    Liara_DescriptorAllocator::Liara_DescriptorAllocator(Liara_Device& device,
                                                         const uint32_t maxSets,
                                                         const VkDescriptorPoolCreateFlags flags,
                                                         const std::vector<VkDescriptorPoolSize>& poolSizes)
        : m_Device(device)
        , m_PoolSizes(poolSizes)
        , m_MaxSets(maxSets)
        , m_Flags(flags) {}

    Liara_DescriptorAllocator::~Liara_DescriptorAllocator() {
        for (auto* const pool : m_FreePools) { vkDestroyDescriptorPool(m_Device.GetDevice(), pool, nullptr); }
        for (auto* const pool : m_UsedPools) { vkDestroyDescriptorPool(m_Device.GetDevice(), pool, nullptr); }
    }

    VkDescriptorPool Liara_DescriptorAllocator::GrabPool() {
        if (!m_FreePools.empty()) {
            VkDescriptorPool pool = m_FreePools.back();
            m_FreePools.pop_back();
            return pool;
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(m_PoolSizes.size());
        poolInfo.pPoolSizes = m_PoolSizes.data();
        poolInfo.maxSets = m_MaxSets;
        poolInfo.flags = m_Flags;

        VkDescriptorPool descriptorPool = nullptr;
        if (vkCreateDescriptorPool(m_Device.GetDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            LIARA_THROW_RUNTIME_ERROR(LogVulkan, "Failed to create descriptor pool");
        }

        return descriptorPool;
    }

    bool Liara_DescriptorAllocator::Allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout) {
        if (m_CurrentPool == VK_NULL_HANDLE) {
            m_CurrentPool = GrabPool();
            m_UsedPools.push_back(m_CurrentPool);
        }

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_CurrentPool;
        allocInfo.pSetLayouts = &layout;
        allocInfo.descriptorSetCount = 1;

        VkResult result = vkAllocateDescriptorSets(m_Device.GetDevice(), &allocInfo, set);
        if (result == VK_SUCCESS) { return true; }
        if (result == VK_ERROR_FRAGMENTED_POOL || result == VK_ERROR_OUT_OF_POOL_MEMORY) {
            m_CurrentPool = GrabPool();
            m_UsedPools.push_back(m_CurrentPool);

            result = vkAllocateDescriptorSets(m_Device.GetDevice(), &allocInfo, set);
            return result == VK_SUCCESS;
        }
        return false;
    }

    void Liara_DescriptorAllocator::ResetPools() {
        for (auto* pool : m_UsedPools) {
            vkResetDescriptorPool(m_Device.GetDevice(), pool, 0);
            m_FreePools.push_back(pool);
        }
        m_UsedPools.clear();
        m_CurrentPool = VK_NULL_HANDLE;
    }

    // *************** Descriptor Layout Cache Builder *********************
    std::unique_ptr<Liara_DescriptorLayoutCache> Liara_DescriptorLayoutCache::Builder::Build() const {
        return std::make_unique<Liara_DescriptorLayoutCache>(m_Device);
    }


    // *************** Descriptor Layout Cache Implementation *********************

    Liara_DescriptorLayoutCache::~Liara_DescriptorLayoutCache() {
        for (const auto& layout : m_LayoutCache | std::views::values) {
            vkDestroyDescriptorSetLayout(m_Device.GetDevice(), layout, nullptr);
        }
    }

    VkDescriptorSetLayout Liara_DescriptorLayoutCache::CreateLayout(const VkDescriptorSetLayoutCreateInfo* info) {
        LayoutInfo key;
        key.bindings.assign(info->pBindings, info->pBindings + info->bindingCount);
        std::ranges::sort(key.bindings,
                          [](const VkDescriptorSetLayoutBinding& first, const VkDescriptorSetLayoutBinding& second) {
                              return first.binding < second.binding;
                          });

        if (const auto it = m_LayoutCache.find(key); it != m_LayoutCache.end()) { return it->second; }

        VkDescriptorSetLayout layout = nullptr;
        if (vkCreateDescriptorSetLayout(m_Device.GetDevice(), info, nullptr, &layout) != VK_SUCCESS) {
            LIARA_THROW_RUNTIME_ERROR(LogVulkan, "Failed to create descriptor set layout");
        }
        m_LayoutCache[key] = layout;
        return layout;
    }

    bool Liara_DescriptorLayoutCache::LayoutInfo::operator==(const LayoutInfo& other) const {
        if (other.bindings.size() != bindings.size()) { return false; }

        // compare each of the bindings is the same. Bindings are sorted so they will match
        for (size_t i = 0; i < bindings.size(); i++) {
            if (other.bindings[i].binding != bindings[i].binding) { return false; }
            if (other.bindings[i].descriptorType != bindings[i].descriptorType) { return false; }
            if (other.bindings[i].descriptorCount != bindings[i].descriptorCount) { return false; }
            if (other.bindings[i].stageFlags != bindings[i].stageFlags) { return false; }
        }
        return true;
    }

    size_t Liara_DescriptorLayoutCache::LayoutInfo::hash() const {
        size_t result = std::hash<size_t>()(bindings.size());
        for (const auto& binding : bindings) {
            const size_t bindingHash = binding.binding | (binding.descriptorType << 8) | (binding.descriptorCount << 16)
                                       | (binding.stageFlags << 24);
            result ^= std::hash<size_t>()(bindingHash);
        }
        return result;
    }

    // *************** Descriptor Builder Implementation *********************

    Liara_DescriptorBuilder::Liara_DescriptorBuilder(Liara_DescriptorLayoutCache& layoutCache,
                                                     Liara_DescriptorAllocator& allocator)
        : m_Cache(layoutCache)
        , m_Alloc(allocator) {}

    Liara_DescriptorBuilder& Liara_DescriptorBuilder::BindBuffer(const uint32_t binding,
                                                                 const VkDescriptorBufferInfo* bufferInfo,
                                                                 const VkDescriptorType type,
                                                                 const VkShaderStageFlags stageFlags) {
        VkDescriptorSetLayoutBinding newBinding{};
        newBinding.binding = binding;
        newBinding.descriptorType = type;
        newBinding.descriptorCount = 1;
        newBinding.stageFlags = stageFlags;
        newBinding.pImmutableSamplers = nullptr;
        m_Bindings.push_back(newBinding);

        VkWriteDescriptorSet newWrite{};
        newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        newWrite.dstBinding = binding;
        newWrite.descriptorCount = 1;
        newWrite.descriptorType = type;
        newWrite.pBufferInfo = bufferInfo;
        m_Writes.push_back(newWrite);

        return *this;
    }

    Liara_DescriptorBuilder& Liara_DescriptorBuilder::BindImage(const uint32_t binding,
                                                                const VkDescriptorImageInfo* imageInfo,
                                                                const VkDescriptorType type,
                                                                const VkShaderStageFlags stageFlags) {
        VkDescriptorSetLayoutBinding newBinding{};
        newBinding.binding = binding;
        newBinding.descriptorType = type;
        newBinding.descriptorCount = 1;
        newBinding.stageFlags = stageFlags;
        newBinding.pImmutableSamplers = nullptr;
        m_Bindings.push_back(newBinding);

        VkWriteDescriptorSet newWrite{};
        newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        newWrite.dstBinding = binding;
        newWrite.descriptorCount = 1;
        newWrite.descriptorType = type;
        newWrite.pImageInfo = imageInfo;
        m_Writes.push_back(newWrite);

        return *this;
    }

    bool Liara_DescriptorBuilder::Build(VkDescriptorSet& set, VkDescriptorSetLayout& layout) {
        // Build layout first
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(m_Bindings.size());
        layoutInfo.pBindings = m_Bindings.data();

        layout = m_Cache.CreateLayout(&layoutInfo);

        // Allocate descriptor
        if (!m_Alloc.Allocate(&set, layout)) { return false; }

        Overwrite(set);
        return true;
    }

    void Liara_DescriptorBuilder::Overwrite(const VkDescriptorSet& set) {
        for (auto& write : m_Writes) { write.dstSet = set; }
        vkUpdateDescriptorSets(
            m_Alloc.m_Device.GetDevice(), static_cast<uint32_t>(m_Writes.size()), m_Writes.data(), 0, nullptr);
    }
}
