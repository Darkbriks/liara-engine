#include <SDL.h>

#include "Core/Application.h"
#include "Core/ApplicationInfo.h"

#include "Demo/DemoApp.h"

constexpr Liara::Core::ApplicationInfo appInfo = {
    .name = "DemoApp",
    .display_name = "Liara Engine Demo",
    .description = "Liara Engine capabilities demonstration application",
    .version = {0, 2, 0, "beta"},
    .organization = "Darkbriks",
    .website = "https://github.com/Darkbriks/liara-engine",
    .copyright = "© 2025 Darkbriks",
    .build_config = LIARA_BUILD_CONFIG,
    .target_platform = LIARA_TARGET_PLATFORM
};

LIARA_APPLICATION_EX(DemoApp, appInfo);
