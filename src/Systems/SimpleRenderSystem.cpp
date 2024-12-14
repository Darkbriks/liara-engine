#include "SimpleRenderSystem.h"
#include "Core/Liara_GameObject.h"
#include "Core/Components/TransformComponent3d.h"

#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace Liara::Systems
{
    struct SimplePushConstantData
    {
        glm::mat4 modelMatrix{1.0f};
        glm::mat4 normalMatrix{1.0f};
    };

    SimpleRenderSystem::SimpleRenderSystem(Graphics::Liara_Device& device, VkRenderPass render_pass, VkDescriptorSetLayout descriptor_set_layout) : m_Device(device)
    {
        CreatePipelineLayout(descriptor_set_layout);
        CreatePipeline(render_pass);
    }

    SimpleRenderSystem::~SimpleRenderSystem()
    {
        vkDestroyPipelineLayout(m_Device.GetDevice(), m_PipelineLayout, nullptr);
    }

    void SimpleRenderSystem::RenderGameObjects(const Core::FrameInfo &frame_info) const
    {
        m_Pipeline->Bind(frame_info.m_CommandBuffer);

        vkCmdBindDescriptorSets(
            frame_info.m_CommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_PipelineLayout,
            0,
            1,
            &frame_info.m_GlobalDescriptorSet,
            0,
            nullptr
        );

        for (auto& [fst, snd] : frame_info.m_GameObjects)
        {
            auto& obj = snd;
            if (!obj.m_Model) { continue; }
            SimplePushConstantData push{};
            push.modelMatrix = obj.m_Transform.GetMat4();
            push.normalMatrix = obj.m_Transform.GetNormalMatrix();
            vkCmdPushConstants(frame_info.m_CommandBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
            obj.m_Model->Bind(frame_info.m_CommandBuffer);
            obj.m_Model->Draw(frame_info.m_CommandBuffer);
        }
    }

    void SimpleRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout descriptor_set_layout)
    {
        constexpr VkPushConstantRange pushConstantRange{
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData)
        };

        const std::vector<VkDescriptorSetLayout> layouts = {descriptor_set_layout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create pipeline layout!");
        }
    }

    void SimpleRenderSystem::CreatePipeline(VkRenderPass render_pass)
    {
        assert(m_PipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        Graphics::PipelineConfigInfo pipelineConfig{};
        Graphics::Liara_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.m_RenderPass = render_pass;
        pipelineConfig.m_PipelineLayout = m_PipelineLayout;
        m_Pipeline = std::make_unique<Graphics::Liara_Pipeline>(m_Device, "shaders/SimpleShader.vert.spv", "shaders/SimpleShader.frag.spv", pipelineConfig);
    }
}