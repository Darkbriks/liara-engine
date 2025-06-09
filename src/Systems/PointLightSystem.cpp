#include "PointLightSystem.h"
#include "Core/FrameInfo.h"
#include "Core/Liara_GameObject.h"
#include "Core/Components/TransformComponent3d.h"
#include "Graphics/Ubo/GlobalUbo.h"
#include "Graphics/Liara_Pipeline.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <ranges>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <fmt/core.h>

#include "Graphics/SpecConstant/SpecializationConstant.h"

namespace Liara::Systems
{
    struct PointLightPushConstants
    {
        glm::vec4 position{};
        glm::vec4 color{};
        float radius;
    };

    PointLightSystem::PointLightSystem(Graphics::Liara_Device& device,
                                      VkRenderPass render_pass,
                                      VkDescriptorSetLayout descriptor_set_layout,
                                      const Core::SettingsManager& settings_manager)
        : m_Device(device), m_SettingsManager(settings_manager)
    {
        CreatePipelineLayout(descriptor_set_layout);
        CreatePipeline(render_pass);
    }

    PointLightSystem::~PointLightSystem()
    {
        vkDestroyPipelineLayout(m_Device.GetDevice(), m_PipelineLayout, nullptr);
    }

    void PointLightSystem::Update(const Core::FrameInfo& frame_info, Graphics::Ubo::GlobalUbo& ubo)
    {
        const auto rotateLight = glm::rotate(glm::mat4(1.f), frame_info.m_DeltaTime, {0.f, -1.f, 0.f});

        int lightIndex = 0;
        const auto maxLights = m_SettingsManager.GetUInt("graphics.max_lights");
        for (auto &snd: frame_info.m_GameObjects | std::views::values)
        {
            auto& obj = snd;
            if (!obj.m_PointLight) { continue; }
            if (lightIndex >= maxLights) {
                fmt::print(stderr, "Too many point lights (max: {})\n", maxLights);
                break;
            }

            obj.m_Transform.position = glm::vec3(rotateLight * glm::vec4(obj.m_Transform.position, 1.f));

            ubo.pointLights[lightIndex].position = glm::vec4(obj.m_Transform.position, 1.0f);
            ubo.pointLights[lightIndex].color = glm::vec4(obj.m_color, obj.m_PointLight->intensity);
            lightIndex++;
        }
        ubo.numLights = lightIndex;
    }


    void PointLightSystem::Render(const Core::FrameInfo &frame_info) const
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

        for (auto &snd: frame_info.m_GameObjects | std::views::values)
        {
            auto& obj = snd;
            if (!obj.m_PointLight) { continue; }

            PointLightPushConstants push{};
            push.position = glm::vec4(obj.m_Transform.position, 1.0f);
            push.color = glm::vec4(obj.m_color, obj.m_PointLight->intensity);
            push.radius = obj.m_Transform.scale.x;
            vkCmdPushConstants(frame_info.m_CommandBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PointLightPushConstants), &push);

            vkCmdDraw(frame_info.m_CommandBuffer, 6, 1, 0, 0);
        }
    }

    void PointLightSystem::CreatePipelineLayout(VkDescriptorSetLayout descriptor_set_layout)
    {
        constexpr VkPushConstantRange pushConstantRange{
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PointLightPushConstants)
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

    void PointLightSystem::CreatePipeline(VkRenderPass render_pass)
    {
        assert(m_PipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        Graphics::PipelineConfigInfo pipelineConfig{};
        Graphics::Liara_Pipeline::DefaultPipelineConfigInfo(pipelineConfig, m_SettingsManager);
        pipelineConfig.m_BindingDescriptions.clear();
        pipelineConfig.m_AttributeDescriptions.clear();
        pipelineConfig.m_RenderPass = render_pass;
        pipelineConfig.m_PipelineLayout = m_PipelineLayout;
        m_Pipeline = std::make_unique<Graphics::Liara_Pipeline>(m_Device, "shaders/PointLight.vert.spv", "shaders/PointLight.frag.spv", pipelineConfig, m_SettingsManager);
    }
}
