/**
 * @file SpecializationConstant.h
 * @brief Defines the `SpecConstant` class, which encapsulates Vulkan specialization constants.
 *
 * This class is used to manage Vulkan specialization constants data and map entries for shaders.
 * A Vulkan specialization constant is a constant value that can be set at pipeline creation time,
 * to allow for more efficient shaders by removing branches and conditionals.
 */

#pragma once

#include <array>
#include <span>

#include "Core/Liara_SettingsManager.h"

#include <vulkan/vulkan_core.h>

namespace Liara::Graphics::SpecConstant
{
    /**
     * @class SpecConstantRegistry
     * @brief Compile-time registry for specialization constants
     */
    template<typename... Types>
    class SpecConstantRegistry
    {
    public:
        // Initialize with provided values
        static void Initialize(Types... values) noexcept
        {
            if constexpr (ConstantCount > 0) {
                s_Data = {static_cast<uint32_t>(values)...};
                s_Initialized = true;
            }
        }

        // Get map entries at compile time
        static constexpr std::array<VkSpecializationMapEntry, sizeof...(Types)> GetMapEntries() noexcept
        {
            std::array<VkSpecializationMapEntry, ConstantCount> entries{};
            for (size_t i = 0; i < ConstantCount; ++i) {
                entries[i] = VkSpecializationMapEntry{
                    .constantID = static_cast<uint32_t>(i),
                    .offset = static_cast<uint32_t>(i * sizeof(uint32_t)),
                    .size = sizeof(uint32_t)
                };
            }
            return entries;
        }

        // Get specialization info
        static VkSpecializationInfo GetSpecializationInfo() noexcept
        {
            static constexpr auto mapEntries = GetMapEntries();

            return VkSpecializationInfo{
                .mapEntryCount = static_cast<uint32_t>(ConstantCount),
                .pMapEntries = mapEntries.data(),
                .dataSize = sizeof(s_Data),
                .pData = s_Data.data()
            };
        }

        // Get current data array (for debugging/inspection)
        static constexpr std::span<const uint32_t> GetData() noexcept
        {
            return std::span<const uint32_t>{s_Data};
        }

        static constexpr size_t Size() noexcept { return ConstantCount; }
        static bool IsInitialized() noexcept { return s_Initialized; }

    private:
        static constexpr size_t ConstantCount = sizeof...(Types);
        using DataArray = std::array<uint32_t, ConstantCount>;

        static inline DataArray s_Data{};
        static inline bool s_Initialized = false;
    };

    /**
     * @brief Graphics-specific specialization constants
     */
    using GraphicsSpecConstants = SpecConstantRegistry<uint32_t>; // MAX_LIGHTS

    /**
     * @class SpecConstant
     * @brief Main interface for specialization constants with settings injection
     */
    class SpecConstant
    {
    public:
        // Initialize with settings manager
        static void Initialize(const Core::SettingsManager& settingsManager)
        {
            GraphicsSpecConstants::Initialize(settingsManager.GetUInt("graphics.max_lights"));
        }

        // Get specialization info for graphics shaders
        static VkSpecializationInfo GetSpecializationInfo()
        {
            if (!GraphicsSpecConstants::IsInitialized()) {
                throw std::runtime_error("Graphics specialization context is not initialized");
            }
            return GraphicsSpecConstants::GetSpecializationInfo();
        }

        // Utility methods for specific constants
        static uint32_t GetMaxLights()
        {
            if (!GraphicsSpecConstants::IsInitialized()) {
                throw std::runtime_error("Graphics specialization context is not initialized");
            }
            const auto data = GraphicsSpecConstants::GetData();
            return data.empty() ? 10 : data[0];
        }
    };
}