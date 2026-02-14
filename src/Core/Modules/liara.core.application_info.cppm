module;

#include <string>
#include <string_view>

export module liara.core.application_info;

import liara.core.version;

export namespace Liara::Core
{
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

        /**
         * @brief Checks if ApplicationInfo is valid
         */
        [[nodiscard]] bool IsValid() const noexcept {
            return !name.empty() && version.IsValid();
        }
    };

    /**
     * @brief Checks if ApplicationInfo is valid
     */
    constexpr bool IsValidAppInfo(const ApplicationInfo& info) noexcept {
        return !info.name.empty() && IsValidVersion(info.version);
    }
}