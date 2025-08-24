#include "Logger.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <regex>
#include <sstream>

#include "LogMacros.h"
#include "UI/ImGuiLogConsole.h"

namespace Liara
{
    LIARA_DEFINE_LOG_CATEGORY(LogCore, "Core", Info, Verbose);
    LIARA_DEFINE_LOG_CATEGORY(LogGraphics, "Graphics", Info, Verbose);
    LIARA_DEFINE_LOG_CATEGORY(LogVulkan, "Vulkan", Warning, Debug);
    LIARA_DEFINE_LOG_CATEGORY(LogRendering, "Rendering", Info, Verbose);
    LIARA_DEFINE_LOG_CATEGORY(LogPlatform, "Platform", Info, Verbose);
    LIARA_DEFINE_LOG_CATEGORY(LogSystems, "Systems", Info, Verbose);
}

namespace Liara::Logging
{
    Logger::Logger() {
        m_worker_thread = std::jthread([this](const std::stop_token& stopToken) { WorkerThread(stopToken); });
    }

    Logger::~Logger() { Shutdown(); }

    Logger& Logger::GetInstance() {
        std::call_once(s_init_flag, []() { s_instance = std::unique_ptr<Logger>(new Logger()); });
        return *s_instance;
    }

    void Logger::Rotate(const std::string& logFilePath) {
        if (std::filesystem::exists(logFilePath)) {
            if (std::ifstream file(logFilePath); file.is_open()) {
                if (std::string firstLine; std::getline(file, firstLine)) {
                    const std::regex timestampRegex(R"(\[(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3})\])");
                    if (std::smatch match; std::regex_search(firstLine, match, timestampRegex) && match.size() > 1) {
                        std::string timestamp = match[1].str();
                        const std::string newFileName =
                            std::format("{}.{}{}",
                                        logFilePath.substr(0, logFilePath.find_last_of('.')),
                                        timestamp,
                                        logFilePath.substr(logFilePath.find_last_of('.')));
                        std::filesystem::rename(logFilePath, newFileName);
                        std::cout << "Log file renamed to: " << newFileName << std::endl;
                    }
                    else { std::cerr << "Failed to extract timestamp from first log line." << std::endl; }
                }
                else { std::cerr << "Failed to read first line from log file: " << logFilePath << std::endl; }
                file.close();
            }
            else { std::cerr << "Failed to open existing log file for renaming: " << logFilePath << std::endl; }
        }

        std::vector<std::string> oldFiles;
        for (const auto& entry : std::filesystem::directory_iterator(".")) {
            if (entry.is_regular_file() && entry.path().extension() == ".log") {
                auto lastWriteTime = std::filesystem::last_write_time(entry.path());
                auto now = std::filesystem::file_time_type::clock::now();
                if (auto age = std::chrono::duration_cast<std::chrono::hours>(now - lastWriteTime).count();
                    age > 24 * 7) {
                    // Keep files older than 7 days
                    oldFiles.push_back(entry.path().string());
                }
            }
        }
        for (const auto& old_file : oldFiles) {
            if (std::filesystem::remove(old_file)) { std::cout << "Deleted old log file: " << old_file << std::endl; }
            else { std::cerr << "Failed to delete old log file: " << old_file << std::endl; }
        }
    }

    void Logger::Initialize(const std::string& logFilePath) {
        auto& instance = GetInstance();

        Rotate(logFilePath);

        instance.m_log_file.open(logFilePath, std::ios::out | std::ios::app);
        if (!instance.m_log_file.is_open()) {
            std::cerr << "Failed to open log file: " << logFilePath << std::endl;
            instance.m_file_output.store(false);
        }

        instance.Log(LogMessage{LogLevel::Info, &LogCore, std::format("Logging initialized - File: {}", logFilePath)});
    }

    void Logger::Shutdown() {
        auto& instance = GetInstance();

        // NOTE: The worker thread will process any remaining messages before shutdown
        if (instance.m_worker_thread.joinable()) {
            instance.m_worker_thread.request_stop();
            instance.m_worker_thread.join();
        }

        if (instance.m_log_file.is_open()) { instance.m_log_file.close(); }
    }

    void Logger::EnableImGuiConsole() {
        if (auto& instance = GetInstance(); !instance.m_gui_console) {
            instance.m_gui_console = std::make_unique<UI::ImGuiLogConsole>();
        }
    }

    void Logger::Log(LogMessage message) { m_message_queue.enqueue(std::move(message)); }

    void Logger::WorkerThread(const std::stop_token& stopToken) {
        LIARA_LOG_VERBOSE(LogCore, "Logger worker thread started.");
        while (!stopToken.stop_requested()) {
            if (auto optionalMessage = m_message_queue.wait_and_dequeue(std::chrono::milliseconds(100));
                optionalMessage.has_value()) {
                ProcessMessage(optionalMessage.value());
            }
        }

        LIARA_LOG_VERBOSE(LogCore, "Logger worker thread stopping, processing remaining messages.");

        // Process any remaining messages in the queue before shutdown
        LogMessage message;
        while (m_message_queue.dequeue(message)) { ProcessMessage(message); }
    }

    void Logger::ProcessMessage(const LogMessage& message) {
        if (m_console_output.load()) {
            const auto formatted = FormatMessage(message, m_color_output.load());

            static std::mutex consoleMutex;
            const std::lock_guard<std::mutex> lock(consoleMutex);

            if (message.level >= LogLevel::Error) { std::cerr << formatted << std::endl; }
            else { std::cout << formatted << std::endl; }
        }

        if (m_file_output.load() && m_log_file.is_open()) {
            const auto formatted = FormatMessage(message, false);  // No color in file output

            static std::mutex fileMutex;
            const std::lock_guard<std::mutex> lock(fileMutex);
            m_log_file << formatted << std::endl;
            m_log_file.flush();
        }

        if (m_gui_console) {
            const std::string formattedTime = FormatTimestamp(message.timestamp);
            m_gui_console->AddLogEntry(message, formattedTime);
        }
    }

    std::string Logger::FormatMessage(const LogMessage& message, const bool useColor) {
        std::ostringstream oss;

        // Begin color escape sequence if enabled
        if (useColor) { oss << LogLevelToColor(message.level); }

        // Timestamp
        oss << "[" << FormatTimestamp(message.timestamp) << "]";

        // Log level
        oss << "[" << LogLevelToString(message.level) << "]";

        // Category
        oss << "[" << message.category->name << "]";

        // Thread ID
        std::ostringstream threadStream;
        threadStream << message.thread_id;
        std::string threadStr = threadStream.str();
        if (threadStr.length() > 6) { threadStr = threadStr.substr(threadStr.length() - 6); }
        oss << "[T:" << threadStr << "]";

        // Class name and function name
        if (m_print_class_name.load()) {
            if (const std::string className = ExtractClassName(message.location.function_name()); !className.empty()) {
                oss << "[" << className << "::" << message.location.function_name() << "]";
            }
            else { oss << "[" << message.location.function_name() << "]"; }
        }

        // Position in the file
        if (m_print_position.load()) {
            oss << "[" << message.location.file_name() << ":" << message.location.line() << "]";
        }

        // Reset color if enabled
        if (useColor) { oss << "\033[0m"; }

        // Message
        oss << " " << message.message;

        return oss.str();
    }

    std::string Logger::FormatTimestamp(const std::chrono::system_clock::time_point& timestamp) {
        auto timeTStamp = std::chrono::system_clock::to_time_t(timestamp);

        timeTStamp += static_cast<std::time_t>(m_timezone_offset_hours.load() * 3600);

        const auto milliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()) % 1000;

        std::tm tmBuf{};
#if defined(_WIN32) || defined(_WIN64)
        gmtime_s(&tmBuf, &timeTStamp);  // Windows
#else
        gmtime_r(&timeTStamp, &tmBuf);  // Linux / macOS
#endif

        std::ostringstream oss;
        oss << std::put_time(&tmBuf, "%Y-%m-%d %H:%M:%S");
        oss << "." << std::setfill('0') << std::setw(3) << milliseconds.count();

        return oss.str();
    }

    std::string Logger::ExtractClassName(const std::string_view functionName) {
        const std::string funcStr{functionName};

        // Chercher le dernier "::" avant la fonction
        const size_t lastScope = funcStr.rfind("::");
        if (lastScope == std::string::npos) {
            return "";  // Pas de classe
        }

        // Chercher l'avant-dernier "::"

        const size_t secondLastScope = funcStr.rfind("::", lastScope - 1);
        if (secondLastScope == std::string::npos) {
            // Format: "Class::Method"
            return funcStr.substr(0, lastScope);
        }
        // Format: "Namespace::Class::Method"
        return funcStr.substr(secondLastScope + 2, lastScope - secondLastScope - 2);
    }

}