//
// Created by antoi on 20/10/2024.
//

#pragma once
#include <memory>

#include "Liara_Device.h"
#include "Liara_GameObject.h"
#include "Liara_Pipeline.h"

namespace Liara
{
    class SimpleRenderSystem
    {
    public:
        SimpleRenderSystem(Liara_Device& device, VkRenderPass render_pass);
        ~SimpleRenderSystem();
        SimpleRenderSystem(const SimpleRenderSystem&) = delete;
        SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

        void RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<Liara_GameObject> &game_objects) const;

    private:
        void CreatePipelineLayout();
        void CreatePipeline(VkRenderPass render_pass);

        Liara_Device& m_Device;
        std::unique_ptr<Liara_Pipeline> m_Pipeline;
        VkPipelineLayout m_PipelineLayout;
    };
} // Liara
