#ifdef LIARA_MODULES_ENABLED
module;
#endif

#include "Core/Logging/LogMacros.h"
#include "Graphics/VkResultToString.h"

#ifndef LIARA_MODULES_ENABLED

    #include <Liara/Utils.h>

#else

    #if defined(_WIN32) || defined(_WIN64)
export module liara.core.utils;
    #else
module liara.core.utils;
    #endif

#endif

namespace Liara::Core
{
    void CheckVkResult(const VkResult res) {
        VK_CHECK(res, "Vulkan operation failed with error: {}", Liara::Graphics::VkResultToString(res));
    }

    bool IsVkSuccess(const VkResult res) noexcept { return res == VK_SUCCESS; }
}