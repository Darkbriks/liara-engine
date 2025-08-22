#pragma once
#include <cstdint>
#include <string>
#include <string_view>

namespace Liara::Core
{

    /**
     * @brief Version information with semantic versioning
     */
    struct Version
    {
        uint32_t major = 0;
        uint32_t minor = 1;
        uint32_t patch = 0;
        std::string_view prerelease = "";

        /**
         * @brief Packs version into a single uint32_t (Vulkan style)
         * Format: MAJOR(10 bits) | MINOR(10 bits) | PATCH(12 bits)
         */
        [[nodiscard]] constexpr uint32_t Packed() const noexcept { return major << 22 | minor << 12 | patch; }

        /**
         * @brief Returns version as string "major.minor.patch[-prerelease]"
         */
        [[nodiscard]] std::string ToString() const {
            std::string result = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
            if (!prerelease.empty()) { result += "-" + std::string(prerelease); }
            return result;
        }

        constexpr bool operator==(const Version& other) const noexcept {
            return major == other.major && minor == other.minor && patch == other.patch;
        }

        constexpr bool operator<(const Version& other) const noexcept {
            if (major != other.major) return major < other.major;
            if (minor != other.minor) return minor < other.minor;
            return patch < other.patch;
        }
    };

    /**
     * @brief Complete application metadata
     */
    struct ApplicationInfo
    {
        // Core information
        std::string_view name = "LiaraApp";
        std::string_view displayName;  // For UI, defaults to name if empty
        std::string_view description = "Application built with Liara Engine";
        Version version{};

        // Organization information
        std::string_view organization;
        std::string_view website;
        std::string_view copyright;

        // Technical information
        std::string_view buildConfig = "Unknown";     // Debug/Release/etc.
        std::string_view targetPlatform = "Unknown";  // Windows/Linux/etc.

        /**
         * @brief Gets display name or falls back to name
         */
        [[nodiscard]] constexpr std::string_view GetDisplayName() const noexcept {
            return displayName.empty() ? name : displayName;
        }

        /**
         * @brief Gets full application title with version
         */
        [[nodiscard]] std::string GetFullTitle() const {
            return std::string(GetDisplayName()) + " v" + version.ToString();
        }
    };

    constexpr bool IsValidVersion(const Version& version) noexcept {
        return version.major < 1024 && version.minor < 1024 && version.patch < 4096;
    }

    constexpr bool IsValidAppInfo(const ApplicationInfo& info) noexcept {
        return !info.name.empty() && IsValidVersion(info.version);
    }
}
