#include "SimpleRenderSystem.h"

#include "Core/Components/TransformComponent3d.h"
#include "Core/FrameInfo.h"
#include "Core/Liara_GameObject.h"
#include "Core/Liara_SettingsManager.h"
#include "Graphics/Liara_Pipeline.h"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

#include "glm/ext/matrix_float4x4.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <ranges>
#include <stdexcept>

namespace Liara::Systems
{
    struct SimplePushConstantData
    {
        glm::mat4 modelMatrix{1.0f};
        glm::mat4 normalMatrix{1.0f};
    };

    SimpleRenderSystem::SimpleRenderSystem(Graphics::Liara_Device& device,
                                           VkRenderPass renderPass,
                                           VkDescriptorSetLayout descriptorSetLayout,
                                           const Core::Liara_SettingsManager& settingsManager)
        : Liara_System("Simple Render System", {.major = 0, .minor = 4, .patch = 2, .prerelease = "dev"})
        , m_Device(device)
        , m_SettingsManager(settingsManager) {
        CreatePipelineLayout(descriptorSetLayout);
        CreatePipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem() {
        vkDestroyPipelineLayout(m_Device.GetDevice(), m_PipelineLayout, nullptr);
    }

    void SimpleRenderSystem::Render(const Core::FrameInfo& frameInfo) const {
        m_Pipeline->Bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_PipelineLayout,
                                0,
                                1,
                                &frameInfo.globalDescriptorSet,
                                0,
                                nullptr);

        for (auto& snd : frameInfo.gameObjects | std::views::values) {
            auto& obj = snd;
            if (!obj.model) { continue; }
            SimplePushConstantData push{};
            push.modelMatrix = obj.transform.GetMat4();
            push.normalMatrix = obj.transform.GetNormalMatrix();
            vkCmdPushConstants(frameInfo.commandBuffer,
                               m_PipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               0,
                               sizeof(SimplePushConstantData),
                               &push);
            obj.model->Bind(frameInfo.commandBuffer);
            obj.model->Draw(frameInfo.commandBuffer);
        }
    }

    void SimpleRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout descriptorSetLayout) {
        constexpr VkPushConstantRange pushConstantRange{
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData)};

        const std::vector<VkDescriptorSetLayout> layouts = {descriptorSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout)
            != VK_SUCCESS) {
            LIARA_THROW_RUNTIME_ERROR(LogSystems, "Failed to create pipeline layout!");
        }
    }

    void SimpleRenderSystem::CreatePipeline(VkRenderPass renderPass) {
        assert(m_PipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        Graphics::PipelineConfigInfo pipelineConfig{};
        Graphics::Liara_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_PipelineLayout;
        m_Pipeline = std::make_unique<Graphics::Liara_Pipeline>(m_Device,
                                                                "shaders/SimpleShader.vert.spv",
                                                                "shaders/SimpleShader.frag.spv",
                                                                pipelineConfig,
                                                                m_SettingsManager);
    }
}