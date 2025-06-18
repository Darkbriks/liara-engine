#pragma once
#include "Liara_System.h"

#include "Core/Liara_SettingsManager.h"

#include <memory>
#include <vulkan/vulkan_core.h>

namespace Liara::Graphics
{
    class Liara_Pipeline;
    class Liara_Device;
}
namespace Liara::Graphics::Ubo
{
    struct GlobalUbo;
}

namespace Liara::Systems
{
    class SimpleRenderSystem final : public Liara_System
    {
    public:
        SimpleRenderSystem(Graphics::Liara_Device& device,
                           VkRenderPass renderPass,
                           VkDescriptorSetLayout descriptorSetLayout,
                           const Core::Liara_SettingsManager& settingsManager);
        ~SimpleRenderSystem() override;

        void Update(const Core::FrameInfo&, Graphics::Ubo::GlobalUbo&) override {}
        void Render(const Core::FrameInfo& frameInfo) const override;

    private:
        void CreatePipelineLayout(VkDescriptorSetLayout descriptorSetLayout);
        void CreatePipeline(VkRenderPass renderPass);

        Graphics::Liara_Device& m_Device;
        std::unique_ptr<Graphics::Liara_Pipeline> m_Pipeline;
        VkPipelineLayout m_PipelineLayout{};

        const Core::Liara_SettingsManager& m_SettingsManager;
    };
}