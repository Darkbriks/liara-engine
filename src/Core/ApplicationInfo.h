#pragma once
#include <string_view>
#include <cstdint>
#include <string>

namespace Liara::Core {
    
    /**
     * @brief Version information with semantic versioning
     */
    struct Version {
        uint32_t major = 0;
        uint32_t minor = 1;
        uint32_t patch = 0;
        std::string_view prerelease = "";

        /**
         * @brief Packs version into a single uint32_t (Vulkan style)
         * Format: MAJOR(10 bits) | MINOR(10 bits) | PATCH(12 bits)
         */
        constexpr uint32_t packed() const noexcept {
            return major << 22 | minor << 12 | patch;
        }
        
        /**
         * @brief Returns version as string "major.minor.patch[-prerelease]"
         */
        std::string to_string() const {
            std::string result = std::to_string(major) + "." + 
                               std::to_string(minor) + "." + 
                               std::to_string(patch);
            if (!prerelease.empty()) {
                result += "-" + std::string(prerelease);
            }
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
    struct ApplicationInfo {
        // Core information
        std::string_view name = "LiaraApp";
        std::string_view display_name = "";  // For UI, defaults to name if empty
        std::string_view description = "Application built with Liara Engine";
        Version version{};
        
        // Organization information
        std::string_view organization = "";
        std::string_view website = "";
        std::string_view copyright = "";
        
        // Technical information
        std::string_view build_config = "Unknown";  // Debug/Release/etc.
        std::string_view target_platform = "Unknown";  // Windows/Linux/etc.
        
        /**
         * @brief Gets display name or falls back to name
         */
        constexpr std::string_view get_display_name() const noexcept {
            return display_name.empty() ? name : display_name;
        }
        
        /**
         * @brief Gets full application title with version
         */
        std::string get_full_title() const {
            return std::string(get_display_name()) + " v" + version.to_string();
        }
    };
    
    constexpr bool is_valid_version(const Version& v) noexcept {
        return v.major < 1024 && v.minor < 1024 && v.patch < 4096;
    }
    
    constexpr bool is_valid_app_info(const ApplicationInfo& info) noexcept {
        return !info.name.empty() && is_valid_version(info.version);
    }
}
