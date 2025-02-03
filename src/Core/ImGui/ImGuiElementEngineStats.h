#pragma once

#include "ImGuiElement.h"
#include "Core/FrameInfo.h"
#include "Graphics/Ubo/GlobalUbo.h"

#include <imgui.h>

namespace Liara::Core::ImGuiElements
{
    class EngineStats : public ImGuiElement
    {
    public:
        EngineStats() = default;

        ~EngineStats() override = default;

        void Draw(const FrameInfo& frame_info, Graphics::Ubo::GlobalUbo& ubo) override
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);

            ImGui::Begin("Engine Stats", nullptr,
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_AlwaysAutoResize);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", frame_info.m_DeltaTime * 1000.0f, 1.0f / frame_info.m_DeltaTime);
            ImGui::Text("Number of Game Objects: %d", frame_info.m_GameObjects.size());
            ImGui::Text("Triangle Count: %d Vertex Count: %d", g_FrameStats.m_PreviousTriangleCount, g_FrameStats.m_PreviousVertexCount);
            ImGui::Text("Draw Call Count: %d", g_FrameStats.m_PreviousDrawCallCount);
            ImGui::Text("Mesh Draw Time: %.3f ms", g_FrameStats.m_PreviousMeshDrawTime);

            // TODO:
            // - Add Plot for Frame Time
            // - Add 1% low
            // - ...

            ImGui::End();
        }
    };
}
