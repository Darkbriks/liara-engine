#pragma once
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string_view>

#include "ApplicationInfo.h"
#include "Logging/Logger.h"
#include "Logging/LogMacros.h"

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

LIARA_DECLARE_LOG_CATEGORY(App, Info, Verbose);

namespace Liara::Core
{
    /**
     * @brief Template to run a Liara application with error handling
     */
    template <typename AppClass> int RunApplication(const ApplicationInfo& appInfo) {
        Liara::Logging::Logger::Initialize("liara_engine.log");

        auto& logger = Liara::Logging::Logger::GetInstance();
        logger.SetConsoleOutput(true);
        logger.SetFileOutput(true);
        logger.SetColorOutput(true);
        logger.SetTimezoneOffset(0);
        logger.SetPrintClassName(false);
        logger.SetPrintLocation(true);

        LIARA_LOG_INFO(App,
                       "Starting application: {} v{}.{}.{}-{}",
                       appInfo.displayName.empty() ? appInfo.name : appInfo.displayName,
                       appInfo.version.major,
                       appInfo.version.minor,
                       appInfo.version.patch,
                       appInfo.version.prerelease.empty() ? "stable" : appInfo.version.prerelease);

        LIARA_LOG_VERBOSE(App,
                          "Application info: {}",
                          std::format("Name: {}, Version: {}.{}.{}-{}, Organization: {}, Website: {}, Copyright: {}, "
                                      "Build Config: {}, Target Platform: {}",
                                      appInfo.name,
                                      appInfo.version.major,
                                      appInfo.version.minor,
                                      appInfo.version.patch,
                                      appInfo.version.prerelease,
                                      appInfo.organization,
                                      appInfo.website,
                                      appInfo.copyright,
                                      appInfo.buildConfig,
                                      appInfo.targetPlatform));

        // TEST_LOGGER();

        bool result = EXIT_FAILURE;

        try {
            AppClass app(appInfo);
            app.Run();
            result = app.GetSettingsManager().SaveToFile("settings.cfg", true) ? EXIT_SUCCESS : EXIT_FAILURE;
        }
        catch (const std::exception& e) {
            LIARA_LOG_FATAL(App, "Application error: {}", e.what());
        }
        catch (...) {
            LIARA_LOG_FATAL(App, "Unknown application error occurred");
        }

        LIARA_LOG_INFO(App, "Application finished with exit code: {}", result);

        Logging::Logger::Shutdown();
        return result;
    }

    /**
     * @brief Helper function to create a valid ApplicationInfo
     */
    constexpr ApplicationInfo CreateApplicationInfo(const std::string_view name,
                                                    const uint32_t major,
                                                    const uint32_t minor,
                                                    const uint32_t patch,
                                                    const std::string_view description = "",
                                                    const std::string_view displayName = "",
                                                    const std::string_view organization = "",
                                                    const std::string_view website = "",
                                                    const std::string_view copyright = "",
                                                    const std::string_view prerelease = "") {
        return ApplicationInfo{
            .name = name,
            .displayName = displayName,
            .description = description.empty() ? "Application built with Liara Engine" : description,
            .version = {.major = major, .minor = minor, .patch = patch, .prerelease = prerelease},
            .organization = organization,
            .website = website,
            .copyright = copyright,
            .buildConfig = LIARA_BUILD_CONFIG,
            .targetPlatform = LIARA_TARGET_PLATFORM
        };
    }
}

/**
 * @brief Main macro to define a Liara application
 *
 * Usage:
 * LIARA_APPLICATION(MyApp, "MyApp", 1, 0, 0, "My awesome application");
 */
#define LIARA_APPLICATION(AppClass, name, major, minor, patch, ...)                                           \
    int main(int, char*[]) {                                                                                  \
        constexpr auto app_info = Liara::Core::CreateApplicationInfo(name, major, minor, patch, __VA_ARGS__); \
        static_assert(Liara::Core::IsValidAppInfo(app_info),                                                  \
                      "Invalid application info provided to LIARA_APPLICATION");                              \
        return Liara::Core::RunApplication<AppClass>(app_info);                                               \
    }

/**
 * @brief Extended macro with more control over application info
 */
#define LIARA_APPLICATION_EX(AppClass, app_info_expr)                               \
    int main(int, char*[]) {                                                        \
        constexpr auto app_info = app_info_expr;                                    \
        static_assert(Liara::Core::IsValidAppInfo(app_info),                        \
                      "Invalid application info provided to LIARA_APPLICATION_EX"); \
        return Liara::Core::RunApplication<AppClass>(app_info);                     \
    }

/**
 * @brief Macro for development/test applications
 */
#define LIARA_DEV_APPLICATION(AppClass, name)                                \
    LIARA_APPLICATION(AppClass,                                              \
                      name,                                                  \
                      0,                                                     \
                      0,                                                     \
                      1,                                                     \
                      "Development build",                                   \
                      name " [DEV]",                                         \
                      "Liara Engine Team",                                   \
                      "https://github.com/Darkbriks/LiaraEngine_CPP_VULKAN", \
                      "© 2024 Liara Engine Team")
