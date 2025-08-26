#pragma once

#include <cstdint>

namespace Liara::Graphics::Constants
{
    constexpr VkClearColorValue CLEAR_COLOR_VALUE = {
        {0.1f, 0.1f, 0.1f, 1.0f}
    };

    constexpr uint32_t MAX_LIGHTS = 10u;

    constexpr uint32_t UNIFORM_BUFFER_ALIGNMENT = 256u;
    constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2u;
}