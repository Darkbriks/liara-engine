#pragma once

#include "Liara_Camera.h"

namespace Liara::Core
{
    struct FrameInfo
    {
        int m_FrameIndex;
        float m_DeltaTime;
        VkCommandBuffer m_CommandBuffer;
        Liara_Camera &m_Camera;
        VkDescriptorSet m_GlobalDescriptorSet;
    };
}