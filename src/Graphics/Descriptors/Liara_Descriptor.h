/**
 * @file Liara_Descriptor.h
 * @brief Defines the descriptor allocator, layout cache, and builder classes, which encapsulate Vulkan descriptor management.
 *
 * This class is responsible for creating and managing Vulkan descriptor pools, layouts, and sets.
 */

#pragma once

#include "Graphics/Liara_Device.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace Liara::Graphics::Descriptors
{
    /**
     * @class Liara_DescriptorAllocator
     * @brief Allocator for Vulkan descriptor pools, responsible for allocating descriptor sets.
     */
    class Liara_DescriptorAllocator
    {
    public:
        /**
        * @class Builder
        * @brief Builder class for constructing a Liara_DescriptorAllocator instance.
        */
        class Builder
        {
        public:
            /**
             * @brief Constructor for the Builder.
             * @param device The device for the allocator.
             */
            explicit Builder(Liara_Device& device) : m_Device{device} {}

            /**
             * @brief Adds a pool size to the allocator builder.
             * @param descriptorType The type of the descriptor pool.
             * @param count The number of descriptors.
             * @return The builder instance.
             */
            Builder& AddPoolSize(VkDescriptorType descriptorType, uint32_t count);

            /**
             * @brief Sets the pool flags for the descriptor pool.
             * @param flags The Vulkan flags for creating the pool.
             * @return The builder instance.
             */
            Builder& SetPoolFlags(VkDescriptorPoolCreateFlags flags);

            /**
             * @brief Sets the maximum number of sets in the descriptor pool.
             * @param count The maximum number of sets.
             * @return The builder instance.
             */
            Builder& SetMaxSets(uint32_t count);

            /**
             * @brief Builds the descriptor allocator instance.
             * @return A unique pointer to the newly constructed Liara_DescriptorAllocator.
             */
            [[nodiscard]] std::unique_ptr<Liara_DescriptorAllocator> Build() const;

        private:
            Liara_Device& m_Device;                             ///< The device for the allocator.
            std::vector<VkDescriptorPoolSize> m_PoolSizes{};    ///< The pool sizes for the allocator.
            uint32_t m_MaxSets = 1000;                          ///< The maximum number of sets.
            VkDescriptorPoolCreateFlags m_PoolFlags = 0;        ///< The flags for the descriptor pool.
        };

        /**
         * @brief Constructor for Liara_DescriptorAllocator.
         * @param device The device for the allocator.
         * @param maxSets The maximum number of descriptor sets.
         * @param flags The flags for the descriptor pool.
         * @param poolSizes The pool sizes for the descriptor allocator.
         */
        explicit Liara_DescriptorAllocator(Liara_Device& device, uint32_t maxSets, VkDescriptorPoolCreateFlags flags, const std::vector<VkDescriptorPoolSize>& poolSizes);

        /**
         * @brief Destructor for Liara_DescriptorAllocator.
         */
        ~Liara_DescriptorAllocator();

        Liara_DescriptorAllocator(const Liara_DescriptorAllocator&) = delete;
        Liara_DescriptorAllocator& operator=(const Liara_DescriptorAllocator&) = delete;

        /**
         * @brief Allocate a descriptor set from the current pool.
         * @param set The descriptor set to be allocated.
         * @param layout The descriptor set layout to use for allocation.
         * @return True if allocation is successful, false otherwise.
         */
        bool Allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout);

        /**
         * @brief Reset all descriptor pools and move them to the free pool.
         */
        void ResetPools();

    private:
        /**
         * @brief Grab a descriptor pool from the free pool list or create a new one.
         * @return The Vulkan descriptor pool.
         */
        VkDescriptorPool GrabPool();

        Liara_Device& m_Device;                             ///< The device for the allocator.
        std::vector<VkDescriptorPoolSize> m_PoolSizes;      ///< The pool sizes for the allocator.
        uint32_t m_MaxSets;                                 ///< The maximum number of descriptor sets.
        VkDescriptorPoolCreateFlags m_Flags;                ///< The flags for the descriptor pool.
        VkDescriptorPool m_CurrentPool{VK_NULL_HANDLE};     ///< The current descriptor pool.
        std::vector<VkDescriptorPool> m_FreePools;          ///< The free descriptor pools.
        std::vector<VkDescriptorPool> m_UsedPools;          ///< The used descriptor pools.

        friend class Liara_DescriptorBuilder;               ///< Friend class for descriptor builder.
    };

    /**
     * @class Liara_DescriptorLayoutCache
     * @brief Cache for descriptor set layouts to avoid repeated Vulkan layout creation.
     */
    class Liara_DescriptorLayoutCache
    {
    public:
        /**
         * @class Builder
         * @brief Builder class for constructing a Liara_DescriptorLayoutCache instance.
         */
        class Builder
        {
        public:
            /**
             * @brief Constructor for the Builder.
             * @param device The device for the layout cache.
             */
            explicit Builder(Liara_Device& device) : m_Device{device} {}

            /**
             * @brief Builds the descriptor layout cache.
             * @return A unique pointer to the newly constructed Liara_DescriptorLayoutCache.
             */
            [[nodiscard]] std::unique_ptr<Liara_DescriptorLayoutCache> Build() const;

        private:
            Liara_Device& m_Device;                        ///< The device for the layout cache.
        };

        /**
         * @brief Constructor for Liara_DescriptorLayoutCache.
         * @param device The device for the cache.
         */
        explicit Liara_DescriptorLayoutCache(Liara_Device& device) : m_Device{device} {}

        /**
         * @brief Destructor for Liara_DescriptorLayoutCache.
         */
        ~Liara_DescriptorLayoutCache();

        Liara_DescriptorLayoutCache(const Liara_DescriptorLayoutCache&) = delete;
        Liara_DescriptorLayoutCache& operator=(const Liara_DescriptorLayoutCache&) = delete;

        /**
         * @brief Creates a descriptor layout from the provided create info.
         * @param info The descriptor set layout creation information.
         * @return The Vulkan descriptor set layout.
         */
        VkDescriptorSetLayout CreateLayout(const VkDescriptorSetLayoutCreateInfo* info);

    private:
        /**
         * @struct LayoutInfo
         * @brief Structure to hold descriptor set layout information.
         */
        struct LayoutInfo
        {
            std::vector<VkDescriptorSetLayoutBinding> bindings;     ///< The bindings for the layout.
            bool operator==(const LayoutInfo& other) const;         ///< Equality operator.
            [[nodiscard]] size_t hash() const;                      ///< Hash function.
        };

        /**
         * @struct LayoutHash
         * @brief Hash function for the LayoutInfo structure.
         */
        struct LayoutHash { size_t operator()(const LayoutInfo& k) const { return k.hash(); } };

        Liara_Device& m_Device;                                                                 ///< The device for the cache.
        std::unordered_map<LayoutInfo, VkDescriptorSetLayout, LayoutHash> m_LayoutCache;        ///< The layout cache.

        friend class Liara_DescriptorBuilder;                                                   ///< Friend class for descriptor builder.
    };

    /**
     * @class Liara_DescriptorBuilder
     * @brief Builder for Vulkan descriptor sets.
     */
    class Liara_DescriptorBuilder
    {
    public:
        /**
         * @brief Constructor for Liara_DescriptorBuilder.
         * @param layoutCache The layout cache to use for creating layouts.
         * @param allocator The allocator to use for allocating descriptor sets.
         */
        Liara_DescriptorBuilder(Liara_DescriptorLayoutCache& layoutCache, Liara_DescriptorAllocator& allocator);
        ~Liara_DescriptorBuilder() = default;
        Liara_DescriptorBuilder(const Liara_DescriptorBuilder&) = delete;
        Liara_DescriptorBuilder& operator=(const Liara_DescriptorBuilder&) = delete;

        /**
         * @brief Binds a buffer descriptor to the builder.
         * @param binding The binding index.
         * @param bufferInfo The buffer descriptor information.
         * @param type The descriptor type.
         * @param stageFlags The shader stage flags.
         * @return The builder instance.
         */
        Liara_DescriptorBuilder& BindBuffer(uint32_t binding, const VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);

        /**
         * @brief Binds an image descriptor to the builder.
         * @param binding The binding index.
         * @param imageInfo The image descriptor information.
         * @param type The descriptor type.
         * @param stageFlags The shader stage flags.
         * @return The builder instance.
         */
        Liara_DescriptorBuilder& BindImage(uint32_t binding, const VkDescriptorImageInfo* imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);

        /**
         * @brief Builds the descriptor set using the builder.
         * @param set The descriptor set to be built.
         * @param layout The descriptor set layout to use.
         * @return True if building was successful, false otherwise.
         */
        bool Build(VkDescriptorSet& set, VkDescriptorSetLayout& layout);

        /**
         * @brief Overwrites the current descriptor set with new information.
         * @param set The descriptor set to overwrite.
         */
        void Overwrite(const VkDescriptorSet& set);

    private:
        std::vector<VkWriteDescriptorSet> m_Writes;                 ///< The write descriptor set.
        std::vector<VkDescriptorSetLayoutBinding> m_Bindings;       ///< The descriptor set layout bindings.
        Liara_DescriptorLayoutCache& m_Cache;                       ///< The layout cache.
        Liara_DescriptorAllocator& m_Alloc;                         ///< The descriptor allocator.
    };
}
