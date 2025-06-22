#pragma once

#include "Core/FrameInfo.h"

#include <imgui.h>

#include "ImGuiElement.h"

namespace Liara::Core::ImGuiElements
{
    class EngineStats final : public ImGuiElement
    {
    public:
        explicit EngineStats(const ApplicationInfo& appInfo)
            : m_app_info(appInfo) {}

        ~EngineStats() override = default;

        void Draw(const FrameInfo& frameInfo, Graphics::Ubo::GlobalUbo&) override {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);

            ImGui::Begin("Engine Stats",
                         nullptr,
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

            if (ImGui::CollapsingHeader("Application Info")) {
                ImGui::Text("Name: %s", m_app_info.GetDisplayName().data());
                ImGui::Text("Version: %s", m_app_info.version.ToString().c_str());
                ImGui::Text("Build: %s (%s)", m_app_info.buildConfig.data(), m_app_info.targetPlatform.data());
                if (!m_app_info.organization.empty()) {
                    ImGui::Text("Organization: %s", m_app_info.organization.data());
                }
            }

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                        frameInfo.deltaTime * 1000.0f,
                        1.0f / frameInfo.deltaTime);
            ImGui::Text("Number of Game Objects: %ld", frameInfo.gameObjects.size());
            ImGui::Text("Triangle Count: %ld Vertex Count: %ld",
                        frameStats.previousTriangleCount,
                        frameStats.previousVertexCount);
            ImGui::Text("Draw Call Count: %ld", frameStats.previousDrawCallCount);
            ImGui::Text("Mesh Draw Time: %.3f ms", frameStats.previousMeshDrawTime);

            // TODO:
            // - Add Plot for Frame Time
            // - Add 1% low
            // - ...

            ImGui::End();
        }

    private:
        const ApplicationInfo& m_app_info;
    };
}
