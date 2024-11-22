#pragma once

#include <functional>

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
}