#pragma once

namespace Liara::Core
{
    struct FrameInfo;
}
namespace Liara::Graphics::Ubo
{
    struct GlobalUbo;
}

namespace Liara::Core
{
    class ImGuiElement
    {
    public:
        ImGuiElement() = default;
        virtual ~ImGuiElement() = default;
        ImGuiElement(const ImGuiElement&) = delete;
        ImGuiElement& operator=(const ImGuiElement&) = delete;

        virtual void Draw(const FrameInfo& frameInfo, Graphics::Ubo::GlobalUbo& ubo) = 0;
    };
}
