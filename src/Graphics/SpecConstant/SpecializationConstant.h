#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>

#include "Graphics/Ubo/GlobalUbo.h"

namespace Liara::Graphics::SpecConstant
{
    class SpecConstant
    {
        static constexpr size_t specDataSize = 1;

        static constexpr uint32_t specData[specDataSize] = {
            MAX_LIGHTS
        };

    public:
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

        static VkSpecializationInfo GetSpecializationInfo()
        {
            VkSpecializationInfo specializationInfo{};
            specializationInfo.mapEntryCount = static_cast<uint32_t>(specDataSize);
            specializationInfo.pMapEntries = GetMapEntries().data();
            specializationInfo.dataSize = sizeof(specData);
            specializationInfo.pData = specData;

            return specializationInfo;
        }
    };
}
