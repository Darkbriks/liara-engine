#pragma once
#include "Core/Liara_SettingsManager.h"

#include <vulkan/vulkan_core.h>

#include <cstddef>
#include <memory>
#include <vector>

#include "Liara_System.h"

namespace Liara::Core
{
    class Liara_GameObject;
}
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
    class PointLightSystem final : public Liara_System
    {
    public:
        PointLightSystem(Graphics::Liara_Device& device,
                         VkRenderPass renderPass,
                         VkDescriptorSetLayout descriptorSetLayout,
                         const Core::Liara_SettingsManager& settingsManager);
        ~PointLightSystem() override;

        void Update(const Core::FrameInfo& frameInfo, Graphics::Ubo::GlobalUbo& ubo) override;
        void Render(const Core::FrameInfo& frameInfo) const override;

        void CacheNeedsRebuild() { m_CacheNeedsRebuild = true; }

    private:
        void CreatePipelineLayout(VkDescriptorSetLayout descriptorSetLayout);
        void CreatePipeline(VkRenderPass renderPass);

        void RebuildLightCache(const Core::FrameInfo& frameInfo);
        void UpdateLightCache(const Core::FrameInfo& frameInfo);

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