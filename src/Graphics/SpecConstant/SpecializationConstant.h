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
#include <mutex>
#include <vulkan/vulkan_core.h>

#include "Graphics/GraphicsConstants.h"

#include "Utils/Singleton.h"

namespace Liara::Core { class SettingsManager; }

namespace Liara::Graphics
{
    /**
     * @class SpecConstantRegistry
     * @brief Compile-time registry for specialization constants
     */
    template<typename... Types>
    class SpecConstantRegistry
    {
    public:
        static void Initialize(Types... values) noexcept
        {
            if (s_Initialized) {
                fmt::print(stderr, "Warning: SpecConstantRegistry already initialized\n");
                return;
            }

            if constexpr (ConstantCount > 0) {
                s_Data = {static_cast<uint32_t>(values)...};
                s_Initialized = true;
            }
        }

        static std::array<VkSpecializationMapEntry, sizeof...(Types)> GetMapEntries()
        {
            if (!s_Initialized) { throw std::runtime_error("SpecConstantRegistry not initialized"); }

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

        static VkSpecializationInfo GetSpecializationInfo()
        {
            if (!s_Initialized) {
                //throw std::runtime_error("SpecConstantRegistry not initialized");
            }

            static const auto mapEntries = GetMapEntries();

            return VkSpecializationInfo{
                .mapEntryCount = static_cast<uint32_t>(ConstantCount),
                .pMapEntries = mapEntries.data(),
                .dataSize = sizeof(s_Data),
                .pData = s_Data.data()
            };
        }

        static std::span<const uint32_t> GetData() noexcept
        {
            return std::span<const uint32_t>{s_Data};
        }

        static constexpr size_t Size() noexcept { return ConstantCount; }
        static bool IsInitialized() noexcept { return s_Initialized; }

        static void Reset() noexcept
        {
            s_Initialized = false;
            s_Data = {};
        }

    private:
        static constexpr size_t ConstantCount = sizeof...(Types);
        using DataArray = std::array<uint32_t, ConstantCount>;

        static inline DataArray s_Data{};
        static inline bool s_Initialized = false;
    };

    using GraphicsSpecConstants = SpecConstantRegistry<uint32_t>; // MAX_LIGHTS

    /**
     * @class SpecConstant
     * @brief Main interface for specialization constants with settings injection
     */
    class SpecConstant : public Singleton<SpecConstant>
    {
    public:

        void Initialize(const Core::SettingsManager& settingsManager)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);

            if (m_Initialized) {
                fmt::print(stderr, "Warning: SpecConstant already initialized\n");
                return;
            }

            GraphicsSpecConstants::Initialize(Constants::MAX_LIGHTS);
            m_Initialized = true;
        }

        static void InitializeGlobal(const Core::SettingsManager& settingsManager)
        {
            GetInstance().Initialize(settingsManager);
        }

        static VkSpecializationInfo GetSpecializationInfo()
        {
            auto& instance = GetInstance();
            std::lock_guard<std::mutex> lock(instance.m_Mutex);

            if (!instance.m_Initialized) { throw std::runtime_error("SpecConstant not initialized"); }

            return GraphicsSpecConstants::GetSpecializationInfo();
        }

        static uint32_t GetMaxLights()
        {
            auto& instance = GetInstance();
            std::lock_guard<std::mutex> lock(instance.m_Mutex);

            if (!instance.m_Initialized) { throw std::runtime_error("SpecConstant not initialized"); }

            const auto data = GraphicsSpecConstants::GetData();
            return data.empty() ? 10 : data[0];
        }

        static bool IsInitialized()
        {
            auto& instance = GetInstance();
            std::lock_guard<std::mutex> lock(instance.m_Mutex);
            return instance.m_Initialized;
        }

    private:
        friend class Singleton<SpecConstant>;

        SpecConstant() = default;

        bool m_Initialized = false;
        std::mutex m_Mutex;
    };
}