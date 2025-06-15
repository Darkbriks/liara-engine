#pragma once
#include <iostream>
#include <cstdlib>

#include "ApplicationInfo.h"

#ifdef NDEBUG
    #define LIARA_BUILD_CONFIG "Release"
#else
    #define LIARA_BUILD_CONFIG "Debug"
#endif

#ifdef _WIN32
    #define LIARA_TARGET_PLATFORM "Windows"
#elif defined(__linux__)
    #define LIARA_TARGET_PLATFORM "Linux"
#elif defined(__APPLE__)
    #define LIARA_TARGET_PLATFORM "macOS"
#else
    #define LIARA_TARGET_PLATFORM "Unknown"
#endif

namespace Liara::Core {

    /**
     * @brief Template to run a Liara application with error handling
     */
    template<typename AppClass>
    int RunApplication(const ApplicationInfo& app_info) {
        try {
            AppClass app(app_info);
            app.Run();
            return app.GetSettingsManager().SaveToFile("settings.cfg", true) ?
                EXIT_SUCCESS : EXIT_FAILURE;
        } catch (const std::exception& e) {
            std::cerr << "Application error: " << e.what() << std::endl;
            return EXIT_FAILURE;
        } catch (...) {
            std::cerr << "Unknown application error occurred" << std::endl;
            return EXIT_FAILURE;
        }
    }

    /**
     * @brief Helper function to create a valid ApplicationInfo
     */
    constexpr ApplicationInfo CreateApplicationInfo(
        const std::string_view name,
        const uint32_t major, const uint32_t minor, const uint32_t patch,
        const std::string_view description = "",
        const std::string_view display_name = "",
        const std::string_view organization = "",
        const std::string_view website = "",
        const std::string_view copyright = "",
        const std::string_view prerelease = ""
    ) {
        return ApplicationInfo{
            .name = name,
            .display_name = display_name,
            .description = description.empty() ? "Application built with Liara Engine" : description,
            .version = { .major = major, .minor = minor, .patch = patch, .prerelease = prerelease },
            .organization = organization,
            .website = website,
            .copyright = copyright,
            .build_config = LIARA_BUILD_CONFIG,
            .target_platform = LIARA_TARGET_PLATFORM
        };
    }
}

/**
 * @brief Main macro to define a Liara application
 *
 * Usage:
 * LIARA_APPLICATION(MyApp, "MyApp", 1, 0, 0, "My awesome application");
 */
#define LIARA_APPLICATION(AppClass, name, major, minor, patch, ...) \
    int main(int argc, char* argv[]) { \
        constexpr auto app_info = Liara::Core::CreateApplicationInfo( \
            name, major, minor, patch, __VA_ARGS__ \
        ); \
        static_assert(Liara::Core::is_valid_app_info(app_info), \
            "Invalid application info provided to LIARA_APPLICATION"); \
        return Liara::Core::RunApplication<AppClass>(app_info); \
    }

/**
 * @brief Extended macro with more control over application info
 */
#define LIARA_APPLICATION_EX(AppClass, app_info_expr) \
    int main(int argc, char* argv[]) { \
        constexpr auto app_info = app_info_expr; \
        static_assert(Liara::Core::is_valid_app_info(app_info), \
            "Invalid application info provided to LIARA_APPLICATION_EX"); \
        return Liara::Core::RunApplication<AppClass>(app_info); \
    }

/**
 * @brief Macro for development/test applications
 */
#define LIARA_DEV_APPLICATION(AppClass, name) \
    LIARA_APPLICATION(AppClass, name, 0, 0, 1, \
        "Development build", \
        name " [DEV]", \
        "Liara Engine Team", \
        "https://github.com/Darkbriks/LiaraEngine_CPP_VULKAN", \
        "© 2024 Liara Engine Team")
