#pragma once

#include <memory>
#include <vulkan/vulkan_core.h>

#include "Liara_System.h"

namespace Liara::Graphics { class Liara_Pipeline; class Liara_Device; }
namespace Liara::Graphics::Ubo { struct GlobalUbo; }

namespace Liara::Systems
{
    class PointLightSystem final : public Liara_System
    {
    public:
        PointLightSystem(Graphics::Liara_Device& device, VkRenderPass render_pass, VkDescriptorSetLayout descriptor_set_layout);
        ~PointLightSystem() override;

        void Update(const Core::FrameInfo& frame_info, Graphics::Ubo::GlobalUbo& ubo) override;
        void Render(const Core::FrameInfo &frame_info) const override;

    private:
        void CreatePipelineLayout(VkDescriptorSetLayout descriptor_set_layout);
        void CreatePipeline(VkRenderPass render_pass);

        Graphics::Liara_Device& m_Device;
        std::unique_ptr<Graphics::Liara_Pipeline> m_Pipeline;
        VkPipelineLayout m_PipelineLayout{};
    };
}