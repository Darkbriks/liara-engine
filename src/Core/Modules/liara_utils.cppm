module;

#include <functional>
#include <vulkan/vulkan_core.h>
#include <cstddef>

export module liara.core.utils;

export namespace Liara::Core {

    /**
     * @brief Combine multiple hash values using FNV-like algorithm
     * @tparam T First type to hash
     * @tparam Rest Additional types to hash
     * @param seed Reference to seed value that will be modified
     * @param v First value to hash
     * @param rest Additional values to hash
     */
    template <typename T, typename... Rest>
    constexpr void HashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

        if constexpr (sizeof...(rest) > 0) {
            (HashCombine(seed, rest), ...);
        }
    }

    /**
     * @brief Helper function to create a hash from multiple values
     * @tparam Args Types of values to hash
     * @param args Values to hash
     * @return Combined hash value
     */
    template<typename... Args>
    constexpr std::size_t MakeHash(const Args&... args) {
        std::size_t seed = 0;
        HashCombine(seed, args...);
        return seed;
    }

    /**
     * @brief Check Vulkan result and throw if not VK_SUCCESS
     * @param res Vulkan result to check
     */
    [[maybe_unused]]
    void CheckVkResult(VkResult res);

    /**
     * @brief Safe Vulkan result checker that doesn't throw
     * @param res Vulkan result to check
     * @return true if VK_SUCCESS, false otherwise
     */
    [[nodiscard]] constexpr bool IsVkSuccess(VkResult res) noexcept;

}