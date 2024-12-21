#pragma once

namespace Liara::Core { struct FrameInfo; }
namespace Liara::Graphics::Ubo { struct GlobalUbo; }

namespace Liara::Systems
{
    class Liara_System
    {
    public:
        Liara_System() = default;
        virtual ~Liara_System() = default;
        Liara_System(const Liara_System&) = delete;
        Liara_System& operator=(const Liara_System&) = delete;

        virtual void Update(const Core::FrameInfo& frame_info, Graphics::Ubo::GlobalUbo& ubo) const = 0;
        virtual void Render(const Core::FrameInfo& frame_info) const = 0;
    };
}