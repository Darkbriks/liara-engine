#include "Logger.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <regex>
#include <sstream>

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
    Logger::Logger() { m_worker_thread = std::thread(&Logger::WorkerThread, this); }

    Logger::~Logger() { Shutdown(); }

    Logger& Logger::GetInstance() {
        std::call_once(s_init_flag, []() { s_instance = std::unique_ptr<Logger>(new Logger()); });
        return *s_instance;
    }

    void Logger::Rotate(const std::string& log_file_path) {
        if (std::filesystem::exists(log_file_path)) {
            if (std::ifstream file(log_file_path); file.is_open()) {
                if (std::string first_line; std::getline(file, first_line)) {
                    std::regex timestamp_regex(R"(\[(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3})\])");
                    if (std::smatch match; std::regex_search(first_line, match, timestamp_regex) && match.size() > 1) {
                        std::string timestamp = match[1].str();
                        std::string new_file_name =
                            std::format("{}.{}{}",
                                        log_file_path.substr(0, log_file_path.find_last_of('.')),
                                        timestamp,
                                        log_file_path.substr(log_file_path.find_last_of('.')));
                        std::filesystem::rename(log_file_path, new_file_name);
                        std::cout << "Log file renamed to: " << new_file_name << std::endl;
                    }
                    else { std::cerr << "Failed to extract timestamp from first log line." << std::endl; }
                }
                else { std::cerr << "Failed to read first line from log file: " << log_file_path << std::endl; }
                file.close();
            }
            else { std::cerr << "Failed to open existing log file for renaming: " << log_file_path << std::endl; }
        }

        std::vector<std::string> old_files;
        for (const auto& entry : std::filesystem::directory_iterator(".")) {
            if (entry.is_regular_file() && entry.path().extension() == ".log") {
                auto last_write_time = std::filesystem::last_write_time(entry.path());
                auto now = std::filesystem::file_time_type::clock::now();
                if (auto age = std::chrono::duration_cast<std::chrono::hours>(now - last_write_time).count();
                    age > 24 * 7) {
                    // Keep files older than 7 days
                    old_files.push_back(entry.path().string());
                }
            }
        }
        for (const auto& old_file : old_files) {
            if (std::filesystem::remove(old_file)) { std::cout << "Deleted old log file: " << old_file << std::endl; }
            else { std::cerr << "Failed to delete old log file: " << old_file << std::endl; }
        }
    }

    void Logger::Initialize(const std::string& log_file_path) {
        auto& instance = GetInstance();

        Rotate(log_file_path);

        instance.m_log_file.open(log_file_path, std::ios::out | std::ios::app);
        if (!instance.m_log_file.is_open()) {
            std::cerr << "Failed to open log file: " << log_file_path << std::endl;
            instance.m_file_output.store(false);
        }

        instance.Log(
            LogMessage{LogLevel::Info, &LogCore, std::format("Logging initialized - File: {}", log_file_path)});
    }

    void Logger::Shutdown() {
        auto& instance = GetInstance();

        if (!instance.m_running.load()) { return; }

        instance.m_running.store(false);

        if (instance.m_worker_thread.joinable()) { instance.m_worker_thread.join(); }

        // NOTE: The worker thread will process any remaining messages before shutdown

        if (instance.m_log_file.is_open()) { instance.m_log_file.close(); }
    }

    void Logger::EnableImGuiConsole() {
        if (auto& instance = GetInstance(); !instance.m_gui_console) {
            instance.m_gui_console = std::make_unique<UI::ImGuiLogConsole>();
        }
    }

    void Logger::Log(LogMessage message) {
        if (!m_running.load()) { return; }

        m_message_queue.enqueue(std::move(message));
    }

    void Logger::WorkerThread() {
        while (m_running.load()) {
            if (auto optional_message = m_message_queue.wait_and_dequeue(std::chrono::milliseconds(100));
                optional_message.has_value()) {
                ProcessMessage(optional_message.value());
            }
        }

        // Process any remaining messages in the queue before shutdown
        LogMessage message;
        while (m_message_queue.dequeue(message)) { ProcessMessage(message); }
    }

    void Logger::ProcessMessage(const LogMessage& message) {
        if (m_console_output.load()) {
            const auto formatted = FormatMessage(message, m_color_output.load());

            static std::mutex console_mutex;
            std::lock_guard<std::mutex> lock(console_mutex);

            if (message.level >= LogLevel::Error) { std::cerr << formatted << std::endl; }
            else { std::cout << formatted << std::endl; }
        }

        if (m_file_output.load() && m_log_file.is_open()) {
            const auto formatted = FormatMessage(message, false);  // No color in file output

            static std::mutex file_mutex;
            std::lock_guard<std::mutex> lock(file_mutex);
            m_log_file << formatted << std::endl;
            m_log_file.flush();
        }

        if (m_gui_console) {
            const std::string formatted_time = FormatTimestamp(message.timestamp);
            m_gui_console->AddLogEntry(message, formatted_time);
        }
    }

    std::string Logger::FormatMessage(const LogMessage& message, const bool use_color) {
        std::ostringstream oss;

        // Begin color escape sequence if enabled
        if (use_color) { oss << LogLevelToColor(message.level); }

        // Timestamp
        oss << "[" << FormatTimestamp(message.timestamp) << "]";

        // Log level
        oss << "[" << LogLevelToString(message.level) << "]";

        // Category
        oss << "[" << message.category->name << "]";

        // Thread ID
        std::ostringstream thread_stream;
        thread_stream << message.thread_id;
        std::string thread_str = thread_stream.str();
        if (thread_str.length() > 6) { thread_str = thread_str.substr(thread_str.length() - 6); }
        oss << "[T:" << thread_str << "]";

        // Class name and function name
        if (m_print_class_name.load()) {
            if (const std::string class_name = ExtractClassName(message.location.function_name());
                !class_name.empty()) {
                oss << "[" << class_name << "::" << message.location.function_name() << "]";
            }
            else { oss << "[" << message.location.function_name() << "]"; }
        }

        // Position in the file
        if (m_print_position.load()) {
            oss << "[" << message.location.file_name() << ":" << message.location.line() << "]";
        }

        // Reset color if enabled
        if (use_color) { oss << "\033[0m"; }

        // Message
        oss << " " << message.message;

        return oss.str();
    }

    std::string Logger::FormatTimestamp(const std::chrono::system_clock::time_point& timestamp) {
        auto time_t_stamp = std::chrono::system_clock::to_time_t(timestamp);

        time_t_stamp += m_timezone_offset_hours.load() * 3600;

        const auto milliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()) % 1000;

        std::tm tm_buf{};
#if defined(_WIN32) || defined(_WIN64)
        gmtime_s(&tm_buf, &time_t_stamp);  // Windows
#else
        gmtime_r(&time_t_stamp, &tm_buf);  // Linux / macOS
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
        oss << "." << std::setfill('0') << std::setw(3) << milliseconds.count();

        return oss.str();
    }

    std::string Logger::ExtractClassName(const std::string_view function_name) {
        const std::string func_str{function_name};

        // Chercher le dernier "::" avant la fonction
        const size_t last_scope = func_str.rfind("::");
        if (last_scope == std::string::npos) {
            return "";  // Pas de classe
        }

        // Chercher l'avant-dernier "::"

        if (const size_t second_last_scope = func_str.rfind("::", last_scope - 1);
            second_last_scope == std::string::npos) {
            // Format: "Class::Method"
            return func_str.substr(0, last_scope);
        }
        else {
            // Format: "Namespace::Class::Method"
            return func_str.substr(second_last_scope + 2, last_scope - second_last_scope - 2);
        }
    }

}