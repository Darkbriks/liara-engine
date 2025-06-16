#pragma once

#include "Liara_Device.h"

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace Liara::Graphics
{
    struct PipelineConfigInfo
    {
        PipelineConfigInfo() = default;

        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

        VkSpecializationInfo m_SpecializationInfo{};

        std::vector<VkVertexInputBindingDescription> m_BindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> m_AttributeDescriptions;

        VkPipelineViewportStateCreateInfo m_ViewportInfo{};
        VkPipelineInputAssemblyStateCreateInfo m_InputAssemblyInfo{};
        VkPipelineRasterizationStateCreateInfo m_RasterizationInfo{};
        VkPipelineMultisampleStateCreateInfo m_MultisampleInfo{};
        VkPipelineColorBlendAttachmentState m_ColorBlendAttachment{};
        VkPipelineColorBlendStateCreateInfo m_ColorBlendInfo{};
        VkPipelineDepthStencilStateCreateInfo m_DepthStencilInfo{};
        std::vector<VkDynamicState> m_DynamicStateEnables;
        VkPipelineDynamicStateCreateInfo m_DynamicStateInfo{};
        VkPipelineLayout m_PipelineLayout = nullptr;
        VkRenderPass m_RenderPass = nullptr;
        uint32_t m_Subpass = 0;
    };

    class Liara_Pipeline
    {
    public:
        Liara_Pipeline() = delete;
        Liara_Pipeline(Liara_Device &device,
                              const std::string& vertFilepath,
                              const std::string& fragFilepath,
                              const PipelineConfigInfo& configInfo,
                              const Core::Liara_SettingsManager& settings_manager);
        ~Liara_Pipeline();

        Liara_Pipeline(const Liara_Pipeline&) = delete;
        Liara_Pipeline& operator=(const Liara_Pipeline&) = delete;

        static void DefaultPipelineConfigInfo(PipelineConfigInfo &configInfo);

        void Bind(VkCommandBuffer commandBuffer) const;

    private:
        Liara_Device& m_Device;
        VkPipeline m_GraphicsPipeline{};
        VkShaderModule m_VertShaderModule{};
        VkShaderModule m_FragShaderModule{};

        static std::vector<char> ReadFile(const std::string& filepath);

        void CreateGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);

        void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) const;
    };
}
