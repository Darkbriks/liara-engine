#pragma once

#include <string_view>
#include <type_traits>

#include "LogLevel.h"  // IWYU pragma: keep

namespace Liara::Logging
{

    struct LogCategory
    {
        std::string_view name;
        LogLevel default_level;
        LogLevel compile_level;

        explicit constexpr LogCategory(const std::string_view cat_name,
                                       const LogLevel def_level = LogLevel::Info,
                                       const LogLevel comp_level = LogLevel::Verbose) noexcept
            : name(cat_name)
            , default_level(def_level)
            , compile_level(comp_level) {}

        constexpr bool IsEnabled(const LogLevel /*level*/) const noexcept {
            return true;  // TODO: Implement compile-time check
            // return level >= compile_level && level >= default_level;
        }
    };

#define LIARA_DECLARE_LOG_CATEGORY_EXTERN(CategoryName, DefaultLevel, CompileLevel) \
    extern const ::Liara::Logging::LogCategory CategoryName

#define LIARA_DECLARE_LOG_CATEGORY(CategoryName, DefaultLevel, CompileLevel)                          \
    constexpr ::Liara::Logging::LogCategory CategoryName {                                            \
        #CategoryName, Liara::Logging::LogLevel::DefaultLevel, Liara::Logging::LogLevel::CompileLevel \
    }

#define LIARA_DEFINE_LOG_CATEGORY(CategoryName, Name, DefaultLevel, CompileLevel)            \
    const ::Liara::Logging::LogCategory CategoryName {                                       \
        Name, Liara::Logging::LogLevel::DefaultLevel, Liara::Logging::LogLevel::CompileLevel \
    }

    LIARA_DECLARE_LOG_CATEGORY_EXTERN(LogCore, Info, Verbose);
    LIARA_DECLARE_LOG_CATEGORY_EXTERN(LogGraphics, Info, Verbose);
    LIARA_DECLARE_LOG_CATEGORY_EXTERN(LogVulkan, Warning, Debug);
    LIARA_DECLARE_LOG_CATEGORY_EXTERN(LogRendering, Info, Verbose);

}