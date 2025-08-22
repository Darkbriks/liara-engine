#pragma once

#include "Logger.h"

namespace Liara::Logging
{

    template <typename Category> constexpr bool ShouldLog(LogLevel level, const Category& category) {
        return category.IsEnabled(level);
    }

}

// ===============================================================
// MACROS FOR LOGGING
// ===============================================================

// Generic macro: specify the category and level
#define LIARA_LOG_IMPL(Category, Level, Format, ...)                                                                \
    do {                                                                                                            \
        if constexpr (::Liara::Logging::ShouldLog(::Liara::Logging::LogLevel::Level, Category)) {                   \
            if (Category.IsEnabled(::Liara::Logging::LogLevel::Level)) {                                            \
                ::Liara::Logging::Logger::GetInstance().Log(::Liara::Logging::LogMessage{                           \
                    ::Liara::Logging::LogLevel::Level, &Category, std::format(Format __VA_OPT__(, ) __VA_ARGS__)}); \
            }                                                                                                       \
        }                                                                                                           \
    }                                                                                                               \
    while (0)

// Specific macros for each log level
// Debug logs are disabled in release builds
#ifdef NDEBUG
    #define LIARA_LOG_DEBUG(Category, Format, ...) \
        do {}                                      \
        while (0)

    #define LOG_DEBUG(Format, ...) \
        do {}                      \
        while (0)
#else
    #define LIARA_LOG_DEBUG(Category, Format, ...) LIARA_LOG_IMPL(Category, Debug, Format __VA_OPT__(, ) __VA_ARGS__)

    #define LOG_DEBUG(Format, ...) LIARA_LOG_DEBUG(::Liara::LogCore, Format __VA_OPT__(, ) __VA_ARGS__)
#endif

#define LIARA_LOG_VERBOSE(Category, Format, ...) LIARA_LOG_IMPL(Category, Verbose, Format __VA_OPT__(, ) __VA_ARGS__)
#define LIARA_LOG_INFO(Category, Format, ...) LIARA_LOG_IMPL(Category, Info, Format __VA_OPT__(, ) __VA_ARGS__)
#define LIARA_LOG_WARNING(Category, Format, ...) LIARA_LOG_IMPL(Category, Warning, Format __VA_OPT__(, ) __VA_ARGS__)
#define LIARA_LOG_ERROR(Category, Format, ...) LIARA_LOG_IMPL(Category, Error, Format __VA_OPT__(, ) __VA_ARGS__)
#define LIARA_LOG_FATAL(Category, Format, ...) LIARA_LOG_IMPL(Category, Fatal, Format __VA_OPT__(, ) __VA_ARGS__)

// ===============================================================
// MACROS FOR THROWING EXCEPTIONS WITH LOGGING
// ===============================================================

// Generic macro : specify the exception type and category
#define LIARA_THROW_EXCEPTION_IMPL(ExceptionType, Category, Format, ...)       \
    do {                                                                       \
        std::string message = std::format(Format __VA_OPT__(, ) __VA_ARGS__);  \
        LIARA_LOG_ERROR(Category, "THROWING {}: {}", #ExceptionType, message); \
        throw ExceptionType(message);                                          \
    }                                                                          \
    while (0)

// Specialized macros for common exception types
#define LIARA_THROW_RUNTIME_ERROR(Category, Format, ...) \
    LIARA_THROW_EXCEPTION_IMPL(std::runtime_error, Category, Format __VA_OPT__(, ) __VA_ARGS__)

#define LIARA_THROW_INVALID_ARGUMENT(Category, Format, ...) \
    LIARA_THROW_EXCEPTION_IMPL(std::invalid_argument, Category, Format __VA_OPT__(, ) __VA_ARGS__)

#define LIARA_THROW_LOGIC_ERROR(Category, Format, ...) \
    LIARA_THROW_EXCEPTION_IMPL(std::logic_error, Category, Format __VA_OPT__(, ) __VA_ARGS__)

#define LIARA_THROW_OUT_OF_RANGE(Category, Format, ...) \
    LIARA_THROW_EXCEPTION_IMPL(std::out_of_range, Category, Format __VA_OPT__(, ) __VA_ARGS__)

// Generic macro to throw any exception type
#define LIARA_THROW_CUSTOM(ExceptionType, Category, Format, ...) \
    LIARA_THROW_EXCEPTION_IMPL(ExceptionType, Category, Format __VA_OPT__(, ) __VA_ARGS__)

// ===============================================================
// MACROS FOR ASSERTIONS WITH LOGGING
// ===============================================================

// Assert that logs and throws if the condition is false
#define LIARA_CHECK_IMPL(Condition, ExceptionType, Category, Format, ...)                                           \
    do {                                                                                                            \
        if (!(Condition)) {                                                                                         \
            std::string message = std::format("Check failed: {} - " Format, #Condition __VA_OPT__(, ) __VA_ARGS__); \
            LIARA_LOG_FATAL(Category, "CHECK FAILED: {}", message);                                                 \
            throw ExceptionType(message);                                                                           \
        }                                                                                                           \
    }                                                                                                               \
    while (0)

// Specialized macros for common exception types
#define LIARA_CHECK_RUNTIME(Condition, Category, Format, ...) \
    LIARA_CHECK_IMPL(Condition, std::runtime_error, Category, Format __VA_OPT__(, ) __VA_ARGS__)

#define LIARA_CHECK_ARGUMENT(Condition, Category, Format, ...) \
    LIARA_CHECK_IMPL(Condition, std::invalid_argument, Category, Format __VA_OPT__(, ) __VA_ARGS__)

// ===============================================================
// MACROS FOR VULKAN ERROR HANDLING
// ===============================================================

#define LIARA_VK_CHECK(result, Category, Format, ...)                                                 \
    do {                                                                                              \
        VkResult _result = (result);                                                                  \
        if (_result != VK_SUCCESS) {                                                                  \
            std::string message =                                                                     \
                std::format("Vulkan error: {} - " Format,                                             \
                            ::Liara::Graphics::VkResultToString(_result) __VA_OPT__(, ) __VA_ARGS__); \
            LIARA_LOG_FATAL(Category, "VK_CHECK FAILED: {}", message);                                \
            throw std::runtime_error(message);                                                        \
        }                                                                                             \
    }                                                                                                 \
    while (0)

#define VK_CHECK(result, Format, ...) LIARA_VK_CHECK(result, ::Liara::LogVulkan, Format __VA_OPT__(, ) __VA_ARGS__)

// ===============================================================
// MACRO FOR TESTING
// ===============================================================

#define TEST_LOGGER()                                                                                          \
    LIARA_LOG_INFO(::Liara::LogCore, "Test log message from {} at line {}", __FILE__, __LINE__);               \
    LIARA_LOG_DEBUG(::Liara::LogCore, "Debugging info: {}", std::format("Value: {}", 42));                     \
    LIARA_LOG_WARNING(::Liara::LogCore, "This is a warning!");                                                 \
    LIARA_LOG_ERROR(::Liara::LogCore, "An error occurred: {}", "Something went wrong");                        \
    LIARA_LOG_FATAL(::Liara::LogCore, "Fatal error! Exiting...");                                              \
    LIARA_LOG_VERBOSE(::Liara::LogCore, "Verbose log message for detailed tracing: {}", "Detailed info here"); \
                                                                                                               \
    try {                                                                                                      \
        LIARA_THROW_RUNTIME_ERROR(::Liara::LogCore, "This is a test runtime error");                           \
    }                                                                                                          \
    catch (const std::exception& e) {                                                                          \
        LIARA_LOG_ERROR(::Liara::LogCore, "Caught exception: {}", e.what());                                   \
    }                                                                                                          \
                                                                                                               \
    LIARA_CHECK_RUNTIME(1 + 1 == 2, ::Liara::LogCore, "Math is broken!");                                      \
    try {                                                                                                      \
        LIARA_CHECK_RUNTIME(1 + 1 == 3, ::Liara::LogCore, "Math is really broken!");                           \
    }                                                                                                          \
    catch (const std::exception& e) {                                                                          \
        LIARA_LOG_ERROR(::Liara::LogCore, "Caught check exception: {}", e.what());                             \
    }                                                                                                          \
                                                                                                               \
    try {                                                                                                      \
        VK_CHECK(VK_ERROR_INITIALIZATION_FAILED, "Failed to initialize Vulkan");                               \
    }                                                                                                          \
    catch (const std::exception& e) {                                                                          \
        LIARA_LOG_ERROR(::Liara::LogCore, "Caught Vulkan exception: {}", e.what());                            \
    }
