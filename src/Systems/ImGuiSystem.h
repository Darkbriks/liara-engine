#pragma once

#include "Liara_System.h"
#include "Core/FrameInfo.h"
#include "Core/ImGui/ImGuiElement.h"
#include "Core/ImGui/ImGuiElementDemo.h"
#include "Core/ImGui/ImGuiElementExample.h"
#include "Graphics/Liara_Device.h"
#include "Graphics/Liara_Pipeline.h"

namespace Liara::Systems
{
    class ImGuiSystem final : public Liara_System
    {
    public:
        ImGuiSystem(const Plateform::Liara_Window& window, Graphics::Liara_Device& device, VkRenderPass renderPass, uint32_t imageCount);
        ~ImGuiSystem() override;

        static void NewFrame();

        void Update(const Core::FrameInfo& frame_info, Graphics::Ubo::GlobalUbo& ubo) override;
        void Render(const Core::FrameInfo &frame_info) const override;

        void AddElement(std::unique_ptr<Core::ImGuiElement> element) { m_Elements.push_back(std::move(element)); }
        void AddDemoElement() { m_Elements.push_back(std::make_unique<Core::ImGuiElements::Demo>()); }
        void AddExampleElement() { m_Elements.push_back(std::make_unique<Core::ImGuiElements::Example>()); }

    private:
        static bool IMGUI_INITIALIZED;

        Graphics::Liara_Device& lveDevice;
        VkDescriptorPool descriptorPool;

        std::vector<std::unique_ptr<Core::ImGuiElement>> m_Elements;
    };
}