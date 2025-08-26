#pragma once

#include "Core/FrameInfo.h"

#include <imgui.h>

#include "ImGuiElement.h"
#include "UI/ImGuiEngineStats.h"
#include "UI/ImGuiLogConsole.h"

namespace Liara::Core::ImGuiElements
{
    class MainMenu final : public ImGuiElement
    {
    public:
        explicit MainMenu(const ApplicationInfo& appInfo)
            : m_app_info(appInfo)
            , m_engine_stats_element(new UI::ImGuiEngineStats(m_app_info)) {}

        ~MainMenu() override = default;

        void Draw(const FrameInfo& frameInfo, Graphics::Ubo::GlobalUbo&) override {
            HandleInput();

            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("Debug")) {
                    ImGui::MenuItem("Engine Stats", "Ctrl+E", &m_show_engine_stats);
                    ImGui::MenuItem("Logs Console", "Ctrl+L", &m_show_log_console);

                    if (ImGui::MenuItem("Test Logs")) { TestLogMessages(); }

                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            if (m_show_engine_stats) {
                if (m_engine_stats_element) { m_engine_stats_element->Draw(frameInfo, &m_show_engine_stats); }
            }

            if (m_show_log_console) {
                if (auto* console = Liara::Logging::Logger::GetInstance().GetImGuiConsole()) {
                    console->Draw(&m_show_log_console);
                }
            }
        }

        void HandleInput() {
            if (ImGui::IsKeyPressed(ImGuiKey_F1)) { m_show_engine_stats = !m_show_engine_stats; }

            if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_E)) { m_show_engine_stats = !m_show_engine_stats; }

            if (ImGui::IsKeyPressed(ImGuiKey_F2)) { m_show_log_console = !m_show_log_console; }

            if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_L)) { m_show_log_console = !m_show_log_console; }

            if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_C)) {
                if (auto* console = Liara::Logging::Logger::GetInstance().GetImGuiConsole()) { console->Clear(); }
            }
        }

    private:
        bool m_show_log_console{false};
        bool m_show_engine_stats{false};

        const ApplicationInfo& m_app_info;
        UI::ImGuiEngineStats* m_engine_stats_element{nullptr};

        static void TestLogMessages() { TEST_LOGGER(); }
    };
}
