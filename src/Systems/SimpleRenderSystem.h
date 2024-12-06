#pragma once

#include <memory>
#include <vulkan/vulkan_core.h>

#include "Core/Liara_Camera.h"
#include "Core/Liara_GameObject.h"
#include "Core/FrameInfo.h"
#include "Graphics/Liara_Device.h"
#include "Graphics/Liara_Pipeline.h"

namespace Liara::Systems
{
    class SimpleRenderSystem
    {
    public:
        SimpleRenderSystem(Graphics::Liara_Device& device, VkRenderPass render_pass);
        ~SimpleRenderSystem();
        SimpleRenderSystem(const SimpleRenderSystem&) = delete;
        SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

        void RenderGameObjects(Core::FrameInfo &frame_info, std::vector<Core::Liara_GameObject> &game_objects) const;

    private:
        void CreatePipelineLayout();
        void CreatePipeline(VkRenderPass render_pass);

        Graphics::Liara_Device& m_Device;
        std::unique_ptr<Graphics::Liara_Pipeline> m_Pipeline;
        VkPipelineLayout m_PipelineLayout{};
    };
} // Liara
