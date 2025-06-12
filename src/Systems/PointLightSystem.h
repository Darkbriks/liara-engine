#pragma once

#include <memory>
#include <vulkan/vulkan_core.h>

#include "Liara_System.h"

#include "Core/Liara_SettingsManager.h"

namespace Liara::Core { class Liara_GameObject; }
namespace Liara::Graphics { class Liara_Pipeline; class Liara_Device; }
namespace Liara::Graphics::Ubo { struct GlobalUbo; }

namespace Liara::Systems
{
    class PointLightSystem final : public Liara_System
    {
    public:
        PointLightSystem(Graphics::Liara_Device& device,
                          VkRenderPass render_pass,
                          VkDescriptorSetLayout descriptor_set_layout,
                          const Core::Liara_SettingsManager& settings_manager);
        ~PointLightSystem() override;

        void Update(const Core::FrameInfo& frame_info, Graphics::Ubo::GlobalUbo& ubo) override;
        void Render(const Core::FrameInfo &frame_info) const override;

        void CacheNeedsRebuild() { m_CacheNeedsRebuild = true; }

    private:
        void CreatePipelineLayout(VkDescriptorSetLayout descriptor_set_layout);
        void CreatePipeline(VkRenderPass render_pass);

        void RebuildLightCache(const Core::FrameInfo& frame_info);
        void UpdateLightCache(const Core::FrameInfo& frame_info);

        Graphics::Liara_Device& m_Device;
        std::unique_ptr<Graphics::Liara_Pipeline> m_Pipeline;
        VkPipelineLayout m_PipelineLayout{};

        const Core::Liara_SettingsManager& m_SettingsManager;

        // Use a vector to cache point lights for efficient rendering
        std::vector<Core::Liara_GameObject*> m_CachedPointLights;
        bool m_CacheNeedsRebuild = true;
        size_t m_LastGameObjectCount = 0;
    };
}