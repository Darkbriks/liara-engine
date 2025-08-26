#pragma once

#include <chrono>
#include <source_location>
#include <string>
#include <thread>

#include "LogCategory.h"
#include "LogLevel.h"

namespace Liara::Logging
{

    struct LogMessage
    {
        LogLevel level;
        const LogCategory* category;
        std::chrono::system_clock::time_point timestamp;
        std::string message;
        std::source_location location;
        std::thread::id thread_id;

        LogMessage() noexcept
            : level(LogLevel::Info)
            , category(nullptr)
            , timestamp(std::chrono::system_clock::now())
            , message("")
            , location(std::source_location::current())
            , thread_id(std::this_thread::get_id()) {}

        LogMessage(const LogLevel lvl,
                   const LogCategory* cat,
                   std::string msg,
                   const std::source_location loc = std::source_location::current()) noexcept
            : level(lvl)
            , category(cat)
            , timestamp(std::chrono::system_clock::now())
            , message(std::move(msg))
            , location(loc)
            , thread_id(std::this_thread::get_id()) {}
    };

}