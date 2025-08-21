#pragma once

#include <format>
#include <source_location>

#include "Logger.h"

namespace Liara::Logging
{

    template <typename Category> constexpr bool ShouldLog(LogLevel level, const Category& category) {
        return category.IsEnabled(level);
    }

}

#define LIARA_LOG_IMPL(Category, Level, Format, ...)                                                                 \
    do {                                                                                                             \
        if constexpr (::Liara::Logging::ShouldLog(::Liara::Logging::LogLevel::Level, Category)) {                    \
            if (Category.IsEnabled(::Liara::Logging::LogLevel::Level)) {                                             \
                auto message = std::format(Format __VA_OPT__(, ) __VA_ARGS__);                                       \
                ::Liara::Logging::Logger::GetInstance().Log(                                                         \
                    ::Liara::Logging::LogMessage{::Liara::Logging::LogLevel::Level, &Category, std::move(message)}); \
            }                                                                                                        \
        }                                                                                                            \
    }                                                                                                                \
    while (0)

#ifdef NDEBUG
    #define LIARA_LOG_DEBUG(Category, Format, ...) \
        do {}                                      \
        while (0)

    #define LOG_DEBUG(Format, ...) \
        do {}                      \
        while (0)
#else
    #define LIARA_LOG_DEBUG(Category, Format, ...) LIARA_LOG_IMPL(Category, Debug, Format __VA_OPT__(, ) __VA_ARGS__)

    #define LOG_DEBUG(Format, ...) LIARA_LOG_DEBUG(::Liara::Logging::LogCore, Format __VA_OPT__(, ) __VA_ARGS__)
#endif

#define LIARA_LOG_VERBOSE(Category, Format, ...) LIARA_LOG_IMPL(Category, Verbose, Format __VA_OPT__(, ) __VA_ARGS__)

#define LIARA_LOG_INFO(Category, Format, ...) LIARA_LOG_IMPL(Category, Info, Format __VA_OPT__(, ) __VA_ARGS__)

#define LIARA_LOG_WARNING(Category, Format, ...) LIARA_LOG_IMPL(Category, Warning, Format __VA_OPT__(, ) __VA_ARGS__)

#define LIARA_LOG_ERROR(Category, Format, ...) LIARA_LOG_IMPL(Category, Error, Format __VA_OPT__(, ) __VA_ARGS__)

#define LIARA_LOG_FATAL(Category, Format, ...) LIARA_LOG_IMPL(Category, Fatal, Format __VA_OPT__(, ) __VA_ARGS__)

#define LOG_VERBOSE(Format, ...) LIARA_LOG_VERBOSE(::Liara::Logging::LogCore, Format __VA_OPT__(, ) __VA_ARGS__)
#define LOG_INFO(Format, ...) LIARA_LOG_INFO(::Liara::Logging::LogCore, Format __VA_OPT__(, ) __VA_ARGS__)
#define LOG_WARNING(Format, ...) LIARA_LOG_WARNING(::Liara::Logging::LogCore, Format __VA_OPT__(, ) __VA_ARGS__)
#define LOG_ERROR(Format, ...) LIARA_LOG_ERROR(::Liara::Logging::LogCore, Format __VA_OPT__(, ) __VA_ARGS__)
#define LOG_FATAL(Format, ...) LIARA_LOG_FATAL(::Liara::Logging::LogCore, Format __VA_OPT__(, ) __VA_ARGS__)

#define TEST_LOGGER()                                                                                     \
    LIARA_LOG_INFO(::Liara::Logging::LogCore, "Test log message from {} at line {}", __FILE__, __LINE__); \
    LIARA_LOG_DEBUG(::Liara::Logging::LogCore, "Debugging info: {}", std::format("Value: {}", 42));       \
    LIARA_LOG_WARNING(::Liara::Logging::LogCore, "This is a warning!");                                   \
    LIARA_LOG_ERROR(::Liara::Logging::LogCore, "An error occurred: {}", "Something went wrong");          \
    LIARA_LOG_FATAL(::Liara::Logging::LogCore, "Fatal error! Exiting...");                                \
    LIARA_LOG_VERBOSE(::Liara::Logging::LogCore, "Verbose log message for detailed tracing: {}", "Detailed info here");
