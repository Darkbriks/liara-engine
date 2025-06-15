#pragma once

#include "ImGuiElement.h"
#include "Core/FrameInfo.h"

#include <imgui.h>

namespace Liara::Core::ImGuiElements
{
    class EngineStats : public ImGuiElement
    {
    public:
        explicit EngineStats(const ApplicationInfo& app_info) : app_info(app_info) {}

        ~EngineStats() override = default;

        void Draw(const FrameInfo& frame_info, Graphics::Ubo::GlobalUbo& ubo) override
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);

            ImGui::Begin("Engine Stats", nullptr,
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_AlwaysAutoResize);

            if (ImGui::CollapsingHeader("Application Info")) {
                ImGui::Text("Name: %s", app_info.get_display_name().data());
                ImGui::Text("Version: %s", app_info.version.to_string().c_str());
                ImGui::Text("Build: %s (%s)", app_info.build_config.data(), app_info.target_platform.data());
                if (!app_info.organization.empty()) {
                    ImGui::Text("Organization: %s", app_info.organization.data());
                }
            }

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", frame_info.m_DeltaTime * 1000.0f, 1.0f / frame_info.m_DeltaTime);
            ImGui::Text("Number of Game Objects: %ld", frame_info.m_GameObjects.size());
            ImGui::Text("Triangle Count: %ld Vertex Count: %ld", g_FrameStats.m_PreviousTriangleCount, g_FrameStats.m_PreviousVertexCount);
            ImGui::Text("Draw Call Count: %ld", g_FrameStats.m_PreviousDrawCallCount);
            ImGui::Text("Mesh Draw Time: %.3f ms", g_FrameStats.m_PreviousMeshDrawTime);

            // TODO:
            // - Add Plot for Frame Time
            // - Add 1% low
            // - ...

            ImGui::End();
        }

    private:
        const ApplicationInfo& app_info;
    };
}
