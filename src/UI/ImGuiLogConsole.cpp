#include "ImGuiLogConsole.h"

#include <algorithm>
#include <cstring>
#include <imgui.h>

namespace Liara::UI
{
    void ImGuiLogConsole::AddLogEntry(const Logging::LogMessage& message, const std::string& formatted_timestamp) {
        std::lock_guard<std::mutex> lock(m_buffer_mutex);

        const size_t index = m_buffer_index.load() % MAX_LOG_ENTRIES;

        m_log_buffer[index] = GuiLogEntry(message, formatted_timestamp);

        m_buffer_index.fetch_add(1);
        m_total_logs.fetch_add(1);

        if (const auto level_index = static_cast<size_t>(message.level); level_index < m_level_counts.size()) {
            m_level_counts[level_index]++;
        }

        m_filter_dirty = true;
    }

    void ImGuiLogConsole::Draw(bool* p_open) {
        if (p_open && !*p_open) return;

        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Log Console", p_open)) {
            ImGui::End();
            return;
        }

        DrawFilterBar();
        ImGui::Separator();
        DrawStats();
        ImGui::Separator();
        DrawLogList();
        ImGui::End();
    }

    void ImGuiLogConsole::Clear() {
        std::lock_guard<std::mutex> lock(m_buffer_mutex);

        m_buffer_index.store(0);
        m_total_logs.store(0);
        m_level_counts.fill(0);
        m_filter_dirty = true;
    }

    void ImGuiLogConsole::UpdateFilteredEntries() const {
        if (!m_filter_dirty) return;

        std::lock_guard<std::mutex> lock(m_buffer_mutex);
        m_filtered_entries.clear();

        const size_t total = m_total_logs.load();
        const size_t buffer_size = std::min(total, MAX_LOG_ENTRIES);

        for (size_t i = 0; i < buffer_size; ++i) {
            size_t index;
            if (total > MAX_LOG_ENTRIES) { index = (m_buffer_index.load() + i) % MAX_LOG_ENTRIES; }
            else { index = i; }

            const auto& entry = m_log_buffer[index];
            if (PassesFilters(entry)) { m_filtered_entries.push_back(&entry); }
        }

        m_filter_dirty = false;
    }

    bool ImGuiLogConsole::PassesFilters(const GuiLogEntry& entry) const {
        if (const auto level_index = static_cast<size_t>(entry.level);
            level_index >= m_level_filters.size() || !m_level_filters[level_index]) {
            return false;
        }

        if (m_text_filter[0] != '\0') {
            if (const std::string filter_text(m_text_filter.data());
                entry.message.find(filter_text) == std::string::npos
                && entry.full_formatted.find(filter_text) == std::string::npos) {
                return false;
            }
        }

        if (m_category_filter[0] != '\0') {
            if (const std::string filter_category(m_category_filter.data());
                entry.category_name.find(filter_category) == std::string::npos) {
                return false;
            }
        }

        return true;
    }

    ImVec4 ImGuiLogConsole::GetLogLevelColor(const Logging::LogLevel level) {
        switch (level) {
            case Logging::LogLevel::Verbose: return ImVec4(0.7f, 0.7f, 0.7f, 1.0f);  // Gris clair
            case Logging::LogLevel::Debug: return ImVec4(0.5f, 0.8f, 1.0f, 1.0f);    // Cyan clair
            case Logging::LogLevel::Info: return ImVec4(0.6f, 1.0f, 0.6f, 1.0f);     // Vert clair
            case Logging::LogLevel::Warning: return ImVec4(1.0f, 0.8f, 0.3f, 1.0f);  // Jaune/Orange
            case Logging::LogLevel::Error: return ImVec4(1.0f, 0.4f, 0.4f, 1.0f);    // Rouge clair
            case Logging::LogLevel::Fatal: return ImVec4(1.0f, 0.2f, 0.8f, 1.0f);    // Magenta
            default: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);                          // Blanc
        }
    }

    void ImGuiLogConsole::DrawFilterBar() {
        ImGui::Text("Levels:");
        ImGui::SameLine();

        const ImVec4 level_colors[] = {GetLogLevelColor(Logging::LogLevel::Verbose),
                                       GetLogLevelColor(Logging::LogLevel::Debug),
                                       GetLogLevelColor(Logging::LogLevel::Info),
                                       GetLogLevelColor(Logging::LogLevel::Warning),
                                       GetLogLevelColor(Logging::LogLevel::Error),
                                       GetLogLevelColor(Logging::LogLevel::Fatal)};

        for (size_t i = 0; i < m_level_filters.size(); ++i) {
            if (i > 0) ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_CheckMark, level_colors[i]);

            if (const char* level_names[] = {"Verbose", "Debug", "Info", "Warning", "Error", "Fatal"};
                ImGui::Checkbox(level_names[i], &m_level_filters[i])) {
                m_filter_dirty = true;
            }

            ImGui::PopStyleColor();
        }

        ImGui::Text("Search:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(200.0f);
        if (ImGui::InputText("##text_filter", m_text_filter.data(), FILTER_BUFFER_SIZE)) { m_filter_dirty = true; }

        ImGui::SameLine();
        ImGui::Text("Category:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150.0f);
        if (ImGui::InputText("##category_filter", m_category_filter.data(), FILTER_BUFFER_SIZE)) {
            m_filter_dirty = true;
        }

        if (ImGui::Checkbox("Auto-scroll", &m_auto_scroll)) {}
        ImGui::SameLine();
        if (ImGui::Checkbox("Timestamps", &m_show_timestamps)) {}
        ImGui::SameLine();
        if (ImGui::Checkbox("Thread IDs", &m_show_thread_ids)) {}
        ImGui::SameLine();
        if (ImGui::Checkbox("Localisation", &m_show_location)) {}
        ImGui::SameLine();
        if (ImGui::Checkbox("Categories", &m_show_categories)) {}

        ImGui::SameLine();
        ImGui::Dummy(ImVec2(20, 0));
        ImGui::SameLine();

        if (ImGui::Button("Clear")) { Clear(); }

        ImGui::SameLine();
        if (ImGui::Button("Select All")) {
            std::ranges::fill(m_level_filters, true);
            m_filter_dirty = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("Deselect All")) {
            std::ranges::fill(m_level_filters, false);
            m_filter_dirty = true;
        }
    }

    void ImGuiLogConsole::DrawStats() const {
        const size_t total = m_total_logs.load();
        const size_t in_buffer = GetBufferSize();

        ImGui::Text("Total: %zu logs | In memory: %zu/%zu", total, in_buffer, MAX_LOG_ENTRIES);

        ImGui::SameLine();
        ImGui::Dummy(ImVec2(20, 0));
        ImGui::SameLine();

        const ImVec4 level_colors[] = {GetLogLevelColor(Logging::LogLevel::Verbose),
                                       GetLogLevelColor(Logging::LogLevel::Debug),
                                       GetLogLevelColor(Logging::LogLevel::Info),
                                       GetLogLevelColor(Logging::LogLevel::Warning),
                                       GetLogLevelColor(Logging::LogLevel::Error),
                                       GetLogLevelColor(Logging::LogLevel::Fatal)};

        for (size_t i = 0; i < m_level_counts.size(); ++i) {
            const char* level_abbr[] = {"V", "D", "I", "W", "E", "F"};
            if (i > 0) ImGui::SameLine();

            ImGui::TextColored(level_colors[i], "%s:%d", level_abbr[i], m_level_counts[i]);
        }
    }

    void ImGuiLogConsole::DrawLogList() {
        UpdateFilteredEntries();

        if (ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
            ImGuiListClipper clipper;
            clipper.Begin(static_cast<int>(m_filtered_entries.size()));

            while (clipper.Step()) {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                    const auto* entry = m_filtered_entries[i];

                    const ImVec4 color = GetLogLevelColor(entry->level);

                    std::string display_text;

                    if (m_show_timestamps) { display_text += "[" + entry->formatted_timestamp + "]"; }

                    display_text += "[" + std::string(Logging::LogLevelToString(entry->level)) + "]";

                    if (m_show_categories) { display_text += "[" + entry->category_name + "]"; }
                    if (m_show_thread_ids) { display_text += "[T:" + entry->thread_id + "]"; }
                    if (m_show_location) { display_text += "[" + entry->location + "]"; }

                    display_text += " " + entry->message;

                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    ImGui::TextUnformatted(display_text.c_str());
                    ImGui::PopStyleColor();

                    ImGui::PushID(i);
                    if (ImGui::BeginPopupContextItem("LogLinePopup")) {
                        ImGui::Text("Log: %s", Logging::LogLevelToString(entry->level).data());
                        ImGui::Separator();

                        if (ImGui::MenuItem("Copy message only")) { ImGui::SetClipboardText(entry->message.c_str()); }
                        if (ImGui::MenuItem("Copy full log line")) { ImGui::SetClipboardText(display_text.c_str()); }
                        if (ImGui::MenuItem("Filter by this category")) {
                            std::snprintf(
                                m_category_filter.data(), m_category_filter.size(), "%s", entry->category_name.c_str());
                            m_filter_dirty = true;
                        }
                        ImGui::EndPopup();
                    }
                    ImGui::PopID();
                }
            }

            if (m_auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) { ImGui::SetScrollHereY(1.0f); }
        }

        ImGui::EndChild();
    }
}