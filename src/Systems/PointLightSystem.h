#pragma once

#include <memory>
#include <vulkan/vulkan_core.h>

#include "Core/FrameInfo.h"
#include "Graphics/Liara_Device.h"
#include "Graphics/Liara_Pipeline.h"

namespace Liara::Graphics::Ubo {
    struct GlobalUbo;
}

namespace Liara::Systems
{
    class PointLightSystem
    {
    public:
        PointLightSystem(Graphics::Liara_Device& device, VkRenderPass render_pass, VkDescriptorSetLayout descriptor_set_layout);
        ~PointLightSystem();
        PointLightSystem(const PointLightSystem&) = delete;
        PointLightSystem& operator=(const PointLightSystem&) = delete;

        void Update(const Core::FrameInfo& frame_info, Graphics::Ubo::GlobalUbo& ubo) const;
        void Render(const Core::FrameInfo &frame_info) const;

    private:
        void CreatePipelineLayout(VkDescriptorSetLayout descriptor_set_layout);
        void CreatePipeline(VkRenderPass render_pass);

        Graphics::Liara_Device& m_Device;
        std::unique_ptr<Graphics::Liara_Pipeline> m_Pipeline;
        VkPipelineLayout m_PipelineLayout{};
    };
} // Liara
