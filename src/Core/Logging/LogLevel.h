#pragma once

#include <cstdint>
#include <string_view>

namespace Liara::Logging
{

    enum class LogLevel : uint8_t
    {
        Verbose = 0,
        Debug = 1,
        Info = 2,
        Warning = 3,
        Error = 4,
        Fatal = 5,
        Off = 6
    };

    constexpr std::string_view LogLevelToString(const LogLevel level) noexcept {
        switch (level) {
            case LogLevel::Verbose: return "VERBOSE";
            case LogLevel::Debug: return "DEBUG";
            case LogLevel::Info: return "INFO";
            case LogLevel::Warning: return "WARNING";
            case LogLevel::Error: return "ERROR";
            case LogLevel::Fatal: return "FATAL";
            case LogLevel::Off: return "OFF";
        }
        return "UNKNOWN";
    }

    constexpr std::string_view LogLevelToColor(const LogLevel level) noexcept {
        switch (level) {
            case LogLevel::Verbose: return "\033[37m";  // White
            case LogLevel::Debug: return "\033[36m";    // Cyan
            case LogLevel::Info: return "\033[32m";     // Green
            case LogLevel::Warning: return "\033[33m";  // Yellow
            case LogLevel::Error: return "\033[31m";    // Red
            case LogLevel::Fatal: return "\033[35m";    // Magenta
            case LogLevel::Off: return "\033[0m";       // Reset
        }
        return "\033[0m";
    }

}