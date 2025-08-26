#pragma once

#include "Core/Logging/LogCategory.h"
#include "Core/Logging/LogLevel.h"
#include "Core/Logging/LogMessage.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <mutex>
#include <string>
#include <vector>

#include "imgui.h"

namespace Liara::UI
{
    struct GuiLogEntry
    {
        Logging::LogLevel level;
        std::string category_name;
        std::string formatted_timestamp;
        std::string thread_id;
        std::string location;
        std::string message;
        std::string full_formatted;

        GuiLogEntry() = default;

        GuiLogEntry(const Logging::LogMessage& log_msg, std::string formatted_time)
            : level(log_msg.level)
            , category_name(log_msg.category ? log_msg.category->name : "Unknown")
            , formatted_timestamp(std::move(formatted_time))
            , message(log_msg.message) {
            std::ostringstream thread_stream;
            thread_stream << log_msg.thread_id;
            thread_id = thread_stream.str();
            if (thread_id.length() > 6) { thread_id = thread_id.substr(thread_id.length() - 6); }

            location = std::string(log_msg.location.file_name()) + ":" + std::to_string(log_msg.location.line());

            full_formatted = std::format("[{}][{}][{}][T:{}] {}",
                                         formatted_timestamp,
                                         Logging::LogLevelToString(level),
                                         category_name,
                                         thread_id,
                                         message);
        }
    };

    class ImGuiLogConsole
    {
    private:
#ifdef NDEBUG
        static constexpr size_t MAX_LOG_ENTRIES = 500;
#else
        static constexpr size_t MAX_LOG_ENTRIES = 10000;
#endif
        static constexpr size_t FILTER_BUFFER_SIZE = 256;

        std::array<GuiLogEntry, MAX_LOG_ENTRIES> m_log_buffer;
        std::atomic<size_t> m_buffer_index{0};
        std::atomic<size_t> m_total_logs{0};
        mutable std::mutex m_buffer_mutex;

        bool m_auto_scroll{true};
        bool m_show_timestamps{true};
        bool m_show_thread_ids{false};
        bool m_show_location{false};
        bool m_show_categories{true};

        std::array<bool, 6> m_level_filters{
            {true, true, true, true, true, true}
        };
        std::array<char, FILTER_BUFFER_SIZE> m_text_filter{};
        std::array<char, FILTER_BUFFER_SIZE> m_category_filter{};

        mutable std::vector<const GuiLogEntry*> m_filtered_entries;
        mutable bool m_filter_dirty{true};

        std::array<int, 6> m_level_counts{};

    public:
        ImGuiLogConsole() = default;
        ~ImGuiLogConsole() = default;

        ImGuiLogConsole(const ImGuiLogConsole&) = delete;
        ImGuiLogConsole& operator=(const ImGuiLogConsole&) = delete;
        ImGuiLogConsole(ImGuiLogConsole&&) = delete;
        ImGuiLogConsole& operator=(ImGuiLogConsole&&) = delete;

        void AddLogEntry(const Logging::LogMessage& message, const std::string& formatted_timestamp);
        void Draw(bool* p_open = nullptr);
        void Clear();

        size_t GetTotalLogCount() const { return m_total_logs.load(); }
        size_t GetBufferSize() const { return std::min(m_total_logs.load(), MAX_LOG_ENTRIES); }

    private:
        void UpdateFilteredEntries() const;
        bool PassesFilters(const GuiLogEntry& entry) const;

        static ImVec4 GetLogLevelColor(Logging::LogLevel level);

        void DrawFilterBar();
        void DrawStats() const;
        void DrawLogList();
    };
}