#pragma once

#include "Liara_Camera.h"

#include <vulkan/vulkan.h>

namespace Liara::Core
{
    struct FrameInfo
    {
        int m_FrameIndex;
        float m_DeltaTime;
        VkCommandBuffer m_CommandBuffer;
        Liara_Camera &m_Camera;
    };
}