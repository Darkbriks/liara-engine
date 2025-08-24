#pragma once

// #ifndef LIARA_MODULES_ENABLED
#include "Core/Logging/LogMacros.h"
#include "Graphics/VkResultToString.h"

#include <vulkan/vulkan_core.h>

#include <cstddef>
#include <format>
#include <functional>
#include <stdexcept>

namespace Liara::Core
{

    template <typename T, typename... Rest>
    constexpr void HashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        if constexpr (sizeof...(rest) > 0) { (HashCombine(seed, rest), ...); }
    }

    template <typename... Args> constexpr std::size_t MakeHash(const Args&... args) {
        std::size_t seed = 0;
        HashCombine(seed, args...);
        return seed;
    }

    inline void CheckVkResult(VkResult res) {
        VK_CHECK(res, "Vulkan operation failed with error: {}", Liara::Graphics::VkResultToString(res));
    }

    [[nodiscard]] constexpr bool IsVkSuccess(VkResult res) noexcept { return res == VK_SUCCESS; }
}

// #else
// export import liara.core.utils;
// #endif