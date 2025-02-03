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
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_AlwaysAutoResize);

            ImGui::Text("FPS: %.1f", 1.0f / frame_info.m_DeltaTime);
            ImGui::Text("Frame Time: %.3f ms", frame_info.m_DeltaTime * 1000.0f);
            ImGui::Text("Frame Index: %d", frame_info.m_FrameIndex);
            ImGui::Text("Number of Game Objects: %d", frame_info.m_GameObjects.size());

            // TODO:
            // - Add Triangle Count
            // - Add Vertex Count
            // - Add Draw Call Count
            // - Add Mesh Draw Time
            // - ...

            ImGui::End();
        }
    };
}
