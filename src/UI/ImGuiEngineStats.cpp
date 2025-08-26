#include "ImGuiEngineStats.h"

#include "Core/FrameInfo.h"

#include "imgui.h"

namespace Liara::UI
{
    void ImGuiEngineStats::Draw(const Core::FrameInfo& frameInfo, bool* p_open) const {
        if (p_open && !*p_open) return;

        ImGui::SetNextWindowPos(ImVec2(700, 500), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Engine Stats", p_open)) {
            ImGui::End();
            return;
        }

        if (ImGui::CollapsingHeader("Application Info")) {
            ImGui::Text("App Name: %s", m_app_info.GetDisplayName().data());
            ImGui::Text("App Version: %s", m_app_info.version.ToString().c_str());
            ImGui::Text("Build: %s (%s)", m_app_info.buildConfig.data(), m_app_info.targetPlatform.data());
            if (!m_app_info.organization.empty()) { ImGui::Text("Organization: %s", m_app_info.organization.data()); }
        }

        if (ImGui::CollapsingHeader("Application Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                        frameInfo.deltaTime * 1000.0f,
                        1.0f / frameInfo.deltaTime);
            ImGui::Text("Number of Game Objects: %ld", frameInfo.gameObjects.size());
            ImGui::Text("Triangle Count: %ld Vertex Count: %ld",
                        frameStats.previousTriangleCount,
                        frameStats.previousVertexCount);
            ImGui::Text("Draw Call Count: %ld", frameStats.previousDrawCallCount);
            ImGui::Text("Mesh Draw Time: %.3f ms", frameStats.previousMeshDrawTime);
        }

        // TODO:
        // - Add Plot for Frame Time
        // - Add 1% low
        // - ...

        ImGui::End();
    }
}