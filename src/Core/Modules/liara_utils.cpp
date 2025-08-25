module;

#include "Core/Logging/LogMacros.h"
#include "Graphics/VkResultToString.h"

#if defined(_WIN32) || defined(_WIN64)
export module liara.core.utils;
#else
module liara.core.utils;
#endif

namespace Liara::Core
{
    void CheckVkResult(const VkResult res) {
        LIARA_LOG_DEBUG(LogVulkan, "Using CheckVkResult from liara.core.utils module");
        VK_CHECK(res, "Vulkan operation failed with error: {}", Liara::Graphics::VkResultToString(res));
    }

    bool IsVkSuccess(const VkResult res) noexcept { return res == VK_SUCCESS; }
}