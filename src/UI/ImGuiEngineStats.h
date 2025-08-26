#pragma once

namespace Liara
{
    namespace Core
    {
        struct ApplicationInfo;
        struct FrameInfo;
    }

    namespace UI
    {
        class ImGuiEngineStats
        {
        public:
            explicit ImGuiEngineStats(const Core::ApplicationInfo& appInfo)
                : m_app_info(appInfo) {}
            ~ImGuiEngineStats() = default;

            ImGuiEngineStats(const ImGuiEngineStats&) = delete;
            ImGuiEngineStats& operator=(const ImGuiEngineStats&) = delete;
            ImGuiEngineStats(ImGuiEngineStats&&) = delete;
            ImGuiEngineStats& operator=(ImGuiEngineStats&&) = delete;

            void Draw(const Core::FrameInfo& frameInfo, bool* p_open = nullptr) const;

        private:
            const Core::ApplicationInfo& m_app_info;
        };

    }
}