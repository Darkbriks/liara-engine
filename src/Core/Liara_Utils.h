#pragma once

#include <functional>
#include <vulkan/vulkan.h>
#include <cstdio>
#include <cstdlib>

namespace Liara::Core
{
    // From https://stackoverflow.com/a/57595105
    template <typename T, typename... Rest>
    void HashCombine(std::size_t& seed, const T& v, const Rest&... rest)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (HashCombine(seed, rest), ...);
    }

    [[maybe_unused]] static void CheckVkResult(const VkResult err)
    {
        if (err == 0) return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0) abort();
    }
}