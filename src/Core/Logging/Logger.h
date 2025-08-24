#pragma once

#include <atomic>
#include <chrono>
#include <format>
#include <fstream>
#include <memory>
#include <string>
#include <thread>

#include "LogCategory.h"
#include "LogLevel.h"
#include "LogMessage.h"
#include "ThreadSafeQueue.h"

namespace Liara::UI
{
    class ImGuiLogConsole;
}

namespace Liara::Logging
{

    class Logger
    {
    private:
        static inline std::unique_ptr<Logger> s_instance = nullptr;
        static inline std::once_flag s_init_flag;

        ThreadSafeQueue<LogMessage> m_message_queue;
        std::unique_ptr<UI::ImGuiLogConsole> m_gui_console;
        std::jthread m_worker_thread;
        std::ofstream m_log_file;

        std::atomic<bool> m_console_output{true};
        std::atomic<bool> m_file_output{true};
        std::atomic<bool> m_color_output{true};
        std::atomic<int> m_timezone_offset_hours{0};  // UTC offset
        std::atomic<bool> m_print_class_name{true};   // Whether to print class name in logs
        std::atomic<bool> m_print_position{true};     // Whether to print file and line number in logs

        Logger();

    public:
        ~Logger();

        static Logger& GetInstance();
        static void Rotate(const std::string& logFilePath);
        static void Initialize(const std::string& logFilePath = "engine.log");
        static void Shutdown();
        static void EnableImGuiConsole();

        void SetConsoleOutput(const bool enable) noexcept { m_console_output.store(enable); }
        void SetFileOutput(const bool enable) noexcept { m_file_output.store(enable); }
        void SetColorOutput(const bool enable) noexcept { m_color_output.store(enable); }
        void SetTimezoneOffset(const int hours) noexcept { m_timezone_offset_hours.store(hours); }
        void SetPrintClassName(const bool enable) noexcept { m_print_class_name.store(enable); }
        void SetPrintLocation(const bool enable) noexcept { m_print_position.store(enable); }

        bool IsConsoleOutputEnabled() const noexcept { return m_console_output.load(); }
        bool IsFileOutputEnabled() const noexcept { return m_file_output.load(); }
        bool IsColorOutputEnabled() const noexcept { return m_color_output.load(); }
        int GetTimezoneOffset() const noexcept { return m_timezone_offset_hours.load(); }
        bool IsPrintClassNameEnabled() const noexcept { return m_print_class_name.load(); }
        bool IsPrintLocationEnabled() const noexcept { return m_print_position.load(); }
        UI::ImGuiLogConsole* GetImGuiConsole() { return m_gui_console.get(); }
        const UI::ImGuiLogConsole* GetImGuiConsole() const { return m_gui_console.get(); }

        void Log(LogMessage message);

    private:
        void WorkerThread(const std::stop_token& stopToken);
        void ProcessMessage(const LogMessage& message);
        std::string FormatMessage(const LogMessage& message, bool useColor = false);
        std::string FormatTimestamp(const std::chrono::system_clock::time_point& timestamp);
        static std::string ExtractClassName(std::string_view functionName);
    };

}