#pragma once

#include <cstdint>

namespace Liara::Graphics::Constants
{
    constexpr uint32_t MAX_LIGHTS = 10u;

    constexpr uint32_t UNIFORM_BUFFER_ALIGNMENT = 256u;
    constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2u;
}