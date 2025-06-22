#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include "Liara_Device.h"

namespace Liara::Graphics
{
    struct PipelineConfigInfo
    {
        PipelineConfigInfo() = default;

        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

        VkSpecializationInfo specializationInfo{};

        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

        VkPipelineViewportStateCreateInfo viewportInfo{};
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
        VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
        VkPipelineMultisampleStateCreateInfo multisampleInfo{};
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class Liara_Pipeline
    {
    public:
        Liara_Pipeline() = delete;
        Liara_Pipeline(Liara_Device& device,
                       const std::string& vertFilepath,
                       const std::string& fragFilepath,
                       const PipelineConfigInfo& configInfo,
                       const Core::Liara_SettingsManager& settingsManager);
        ~Liara_Pipeline();

        Liara_Pipeline(const Liara_Pipeline&) = delete;
        Liara_Pipeline& operator=(const Liara_Pipeline&) = delete;

        static void DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

        void Bind(VkCommandBuffer commandBuffer) const;

    private:
        Liara_Device& m_Device;
        VkPipeline m_GraphicsPipeline{};
        VkShaderModule m_VertShaderModule{};
        VkShaderModule m_FragShaderModule{};

        static std::vector<char> ReadFile(const std::string& filepath);

        void CreateGraphicsPipeline(const std::string& vertFilepath,
                                    const std::string& fragFilepath,
                                    const PipelineConfigInfo& configInfo);

        void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) const;
    };
}
