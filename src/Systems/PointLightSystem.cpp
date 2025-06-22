#include "PointLightSystem.h"

#include "Core/Components/TransformComponent3d.h"
#include "Core/FrameInfo.h"
#include "Core/Liara_GameObject.h"
#include "Core/Liara_SettingsManager.h"
#include "Graphics/GraphicsConstants.h"
#include "Graphics/Liara_Pipeline.h"
#include "Graphics/Ubo/GlobalUbo.h"

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float4.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <fmt/core.h>

#include <ranges>
#include <stdexcept>

#include "glm/gtx/rotate_vector.hpp"

namespace Liara::Systems
{
    struct PointLightPushConstants
    {
        glm::vec4 position{};
        glm::vec4 color{};
        float radius;
    };

    PointLightSystem::PointLightSystem(Graphics::Liara_Device& device,
                                       VkRenderPass renderPass,
                                       VkDescriptorSetLayout descriptorSetLayout,
                                       const Core::Liara_SettingsManager& settingsManager)
        : m_Device(device)
        , m_SettingsManager(settingsManager) {
        CreatePipelineLayout(descriptorSetLayout);
        CreatePipeline(renderPass);
    }

    PointLightSystem::~PointLightSystem() { vkDestroyPipelineLayout(m_Device.GetDevice(), m_PipelineLayout, nullptr); }

    void PointLightSystem::Update(const Core::FrameInfo& frameInfo, Graphics::Ubo::GlobalUbo& ubo) {
        UpdateLightCache(frameInfo);

        const auto rotateLight = glm::rotate(glm::mat4(1.f), frameInfo.deltaTime, {0.f, -1.f, 0.f});

        const size_t lightCount =
            std::min(m_CachedPointLights.size(), static_cast<size_t>(Graphics::Constants::MAX_LIGHTS));

        for (size_t i = 0; i < lightCount; ++i) {
            auto* gameObject = m_CachedPointLights[i];

            gameObject->transform.position = glm::vec3(rotateLight * glm::vec4(gameObject->transform.position, 1.f));

            ubo.pointLights[i].position = glm::vec4(gameObject->transform.position, 1.0f);
            ubo.pointLights[i].color = glm::vec4(gameObject->color, gameObject->pointLight->intensity);
        }

        ubo.numLights = static_cast<int>(lightCount);
    }


    void PointLightSystem::Render(const Core::FrameInfo& frameInfo) const {
        if (m_CachedPointLights.empty()) { return; }

        m_Pipeline->Bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_PipelineLayout,
                                0,
                                1,
                                &frameInfo.globalDescriptorSet,
                                0,
                                nullptr);

        for (const auto* gameObject : m_CachedPointLights) {
            PointLightPushConstants push{};
            push.position = glm::vec4(gameObject->transform.position, 1.0f);
            push.color = glm::vec4(gameObject->color, gameObject->pointLight->intensity);
            push.radius = gameObject->transform.scale.x;

            vkCmdPushConstants(frameInfo.commandBuffer,
                               m_PipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               0,
                               sizeof(PointLightPushConstants),
                               &push);

            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        }
    }

    void PointLightSystem::CreatePipelineLayout(VkDescriptorSetLayout descriptorSetLayout) {
        constexpr VkPushConstantRange pushConstantRange{
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PointLightPushConstants)};

        const std::vector<VkDescriptorSetLayout> layouts = {descriptorSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout)
            != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }
    }

    void PointLightSystem::CreatePipeline(VkRenderPass renderPass) {
        assert(m_PipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        Graphics::PipelineConfigInfo pipelineConfig{};
        Graphics::Liara_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.bindingDescriptions.clear();
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_PipelineLayout;
        m_Pipeline = std::make_unique<Graphics::Liara_Pipeline>(
            m_Device, "shaders/PointLight.vert.spv", "shaders/PointLight.frag.spv", pipelineConfig, m_SettingsManager);
    }

    void PointLightSystem::RebuildLightCache(const Core::FrameInfo& frameInfo) {
        m_CachedPointLights.clear();
        m_CachedPointLights.reserve(Graphics::Constants::MAX_LIGHTS);

        for (auto& gameObject : frameInfo.gameObjects | std::views::values) {
            if (gameObject.pointLight) {
                m_CachedPointLights.push_back(&gameObject);

                if (m_CachedPointLights.size() >= Graphics::Constants::MAX_LIGHTS) {
                    fmt::print(
                        stderr, "Warning: Too many point lights, limiting to {}\n", Graphics::Constants::MAX_LIGHTS);
                    break;
                }
            }
        }

        m_LastGameObjectCount = frameInfo.gameObjects.size();
        m_CacheNeedsRebuild = false;
    }

    void PointLightSystem::UpdateLightCache(const Core::FrameInfo& frameInfo) {
        if (m_CacheNeedsRebuild || m_LastGameObjectCount != frameInfo.gameObjects.size()) {
            RebuildLightCache(frameInfo);
        }
    }
}
