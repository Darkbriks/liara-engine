#pragma once

#include "ImGuiElement.h"

#include <imgui.h>

namespace Liara::Core { struct FrameInfo; }
namespace Liara::Graphics::Ubo { struct GlobalUbo; }

namespace Liara::Core::ImGuiElements
{
    class Demo : public ImGuiElement
    {
    public:
        Demo() = default;
        ~Demo() override = default;

        void Draw(const FrameInfo& frame_info, Graphics::Ubo::GlobalUbo& ubo) override
        {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        protected:
            bool show_demo_window = true;
    };
}
