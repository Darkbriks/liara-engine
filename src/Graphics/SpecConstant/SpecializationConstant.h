/**
 * @file SpecializationConstant.h
 * @brief Defines the `SpecConstant` class, which encapsulates Vulkan specialization constants.
 *
 * This class is used to manage Vulkan specialization constants data and map entries for shaders.
 * A Vulkan specialization constant is a constant value that can be set at pipeline creation time,
 * to allow for more efficient shaders by removing branches and conditionals.
 */

#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>

namespace Liara::Graphics::SpecConstant
{
    /**
     * @class SpecConstant
     * @brief Class that encapsulates Vulkan specialization constants.
     */
    class SpecConstant
    {
        static constexpr size_t specDataSize = 1;                           ///< The number of specialization constants.

        static constexpr uint32_t (*specDataPtr[specDataSize])() = {        ///< The array of function pointers to get the specialization constant values.
            []() { return static_cast<uint32_t>(Singleton<Liara_Settings>::GetInstanceSync().GetMaxLights()); }
        };

        static uint32_t SpecData[specDataSize];                             ///< The array of specialization constant values.
        static bool SpecDataInitialized;                                    ///< Whether the specialization constant values have been initialized.

    public:
        /**
         * @brief Returns a vector of `VkSpecializationMapEntry` that maps specialization indices to specific constants.
         *
         * This method generates a list of specialization map entries for each element of `specData`, allowing Vulkan
         * to know where and how to apply specializations to the shader.
         *
         * @return A vector of `VkSpecializationMapEntry` structures.
         */
        static std::vector<VkSpecializationMapEntry> GetMapEntries()
        {
            std::vector<VkSpecializationMapEntry> mapEntries(specDataSize);
            for (size_t i = 0; i < specDataSize; i++)
            {
                mapEntries[i].constantID = static_cast<uint32_t>(i);
                mapEntries[i].offset = static_cast<uint32_t>(i * sizeof(uint32_t));
                mapEntries[i].size = sizeof(uint32_t);
            }

            return mapEntries;
        }

        /**
         * @brief Returns a `VkSpecializationInfo` structure with the specialization constant data.
         *
         * This method generates a `VkSpecializationInfo` structure with the specialization constant data and map entries.
         *
         * @return A `VkSpecializationInfo` structure.
         */
        static VkSpecializationInfo GetSpecializationInfo()
        {
            if (!SpecDataInitialized)
            {
                for (size_t i = 0; i < specDataSize; i++) { SpecData[i] = specDataPtr[i](); }
                SpecDataInitialized = true;
            }

            VkSpecializationInfo specializationInfo{};
            specializationInfo.mapEntryCount = static_cast<uint32_t>(specDataSize);
            specializationInfo.pMapEntries = GetMapEntries().data();
            specializationInfo.dataSize = sizeof(SpecData);
            specializationInfo.pData = SpecData;

            return specializationInfo;
        }
    };

    uint32_t SpecConstant::SpecData[specDataSize];
    bool SpecConstant::SpecDataInitialized = false;
}