#pragma once
#include "Core/ApplicationInfo.h"

namespace Liara::Core
{
    struct FrameInfo;
}
namespace Liara::Graphics::Ubo
{
    struct GlobalUbo;
}

namespace Liara::Systems
{
    class Liara_System
    {
    public:
        Liara_System() = default;
        explicit Liara_System(std::string name, const Core::Version& version)
            : m_Name(std::move(name))
            , m_Version(version) {}

        virtual ~Liara_System() = default;
        Liara_System(const Liara_System&) = delete;
        Liara_System& operator=(const Liara_System&) = delete;

        [[nodiscard]] const std::string& Name() const { return m_Name; }
        [[nodiscard]] const Core::Version& Version() const { return m_Version; }

        virtual void Update(const Core::FrameInfo& frameInfo, Graphics::Ubo::GlobalUbo& ubo) = 0;
        virtual void Render(const Core::FrameInfo& frameInfo) const = 0;

    private:
        std::string m_Name{"UnnamedSystem"};
        Core::Version m_Version{.major = 0, .minor = 0, .patch = 0, .prerelease = "no version"};
    };
}