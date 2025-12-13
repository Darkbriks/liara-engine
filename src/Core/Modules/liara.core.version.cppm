module;

#include <cstdint>
#include <string>
#include <string_view>

export module liara.core.version;

export namespace Liara::Core
{
    constexpr uint32_t majorVersionBits = 10;
    constexpr uint32_t minorVersionBits = 10;
    constexpr uint32_t patchVersionBits = 12;

    /**
     * @brief Version information with semantic versioning
     */
    struct Version
    {
        uint32_t major = 0;
        uint32_t minor = 1;
        uint32_t patch = 0;
        std::string_view prerelease;

        /**
         * @brief Packs version into a single uint32_t (Vulkan style)
         * Format: MAJOR(10 bits) | MINOR(10 bits) | PATCH(12 bits)
         */
        [[nodiscard]] constexpr uint32_t Packed() const noexcept { return major << (minorVersionBits + patchVersionBits) | minor << patchVersionBits | patch; }

        /**
         * @brief Checks if version components are within valid ranges
         */
        [[nodiscard]] bool IsValid() const noexcept {
            return major < (1u << majorVersionBits) &&
                   minor < (1u << minorVersionBits) &&
                   patch < (1u << patchVersionBits);
        }

        /**
         * @brief Returns version as string "major.minor.patch[-prerelease]"
         */
        [[nodiscard]] std::string ToString() const {
            std::string result = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
            if (!prerelease.empty()) { result += "-" + std::string(prerelease); }
            return result;
        }

        constexpr auto operator<=>(const Version& other) const noexcept {
            if (const auto cmp = major <=> other.major; cmp != 0) return cmp;
            if (const auto cmp = minor <=> other.minor; cmp != 0) return cmp;
            if (const auto cmp = patch <=> other.patch; cmp != 0) return cmp;
            return prerelease <=> other.prerelease;
        }

        constexpr bool operator==(const Version&) const noexcept = default;
    };

    /**
     * @brief Checks if Version is valid
     */
    constexpr bool IsValidVersion(const Version& version) noexcept {
        return version.major < (1u << majorVersionBits) &&
               version.minor < (1u << minorVersionBits) &&
               version.patch < (1u << patchVersionBits);
    }
}