#pragma once

#include <imgui.h>

#include "ImGuiElement.h"

namespace Liara::Core
{
    struct FrameInfo;
}
namespace Liara::Graphics::Ubo
{
    struct GlobalUbo;
}

namespace Liara::Core::ImGuiElements
{
    class Demo : public ImGuiElement
    {
    public:
        Demo() = default;
        ~Demo() override = default;

        void Draw(const FrameInfo&, Graphics::Ubo::GlobalUbo&) override { ImGui::ShowDemoWindow(&m_show_demo_window); }

    protected:
        bool m_show_demo_window = true;
    };
}
