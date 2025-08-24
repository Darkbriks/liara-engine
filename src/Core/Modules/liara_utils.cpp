module liara.core.utils;

#include "Core/Logging/LogMacros.h"
#include "Graphics/VkResultToString.h"

namespace Liara::Core
{

    void CheckVkResult(const VkResult res) {
        VK_CHECK(res, "Vulkan operation failed with error: {}", Liara::Graphics::VkResultToString(res));
    }

    bool IsVkSuccess(const VkResult res) noexcept { return res == VK_SUCCESS; }
}