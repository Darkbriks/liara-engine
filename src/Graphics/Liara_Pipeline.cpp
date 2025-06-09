#include "Liara_Pipeline.h"
#include "Liara_Model.h"
#include "SpecConstant/SpecializationConstant.h"

#include <fstream>
#include <stdexcept>
#include <cassert>

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

namespace Liara::Graphics
{

    Liara_Pipeline::Liara_Pipeline(Liara_Device &device,
                                   const std::string &vertFilepath,
                                   const std::string &fragFilepath,
                                   const PipelineConfigInfo &configInfo,
                                   const Core::SettingsManager& settings_manager)
        : m_Device(device)
    {
        SpecConstant::SpecConstant::SpecConstant::Initialize(settings_manager);
        CreateGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
    }

    Liara_Pipeline::~Liara_Pipeline()
    {
        vkDestroyShaderModule(m_Device.GetDevice(), m_VertShaderModule, nullptr);
        vkDestroyShaderModule(m_Device.GetDevice(), m_FragShaderModule, nullptr);
        vkDestroyPipeline(m_Device.GetDevice(), m_GraphicsPipeline, nullptr);
    }

    void Liara_Pipeline::DefaultPipelineConfigInfo(PipelineConfigInfo &configInfo, const Core::SettingsManager& settings_manager)
    {
        configInfo.m_InputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.m_InputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.m_InputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        configInfo.m_ViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        configInfo.m_ViewportInfo.viewportCount = 1;
        configInfo.m_ViewportInfo.scissorCount = 1;
        configInfo.m_ViewportInfo.pViewports = nullptr;
        configInfo.m_ViewportInfo.pScissors = nullptr;

        configInfo.m_RasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        configInfo.m_RasterizationInfo.depthClampEnable = VK_FALSE;
        configInfo.m_RasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        configInfo.m_RasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        configInfo.m_RasterizationInfo.lineWidth = 1.0f;
        configInfo.m_RasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        configInfo.m_RasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        configInfo.m_RasterizationInfo.depthBiasEnable = VK_FALSE;
        configInfo.m_RasterizationInfo.depthBiasConstantFactor = 0.0f;
        configInfo.m_RasterizationInfo.depthBiasClamp = 0.0f;
        configInfo.m_RasterizationInfo.depthBiasSlopeFactor = 0.0f;

        configInfo.m_MultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.m_MultisampleInfo.sampleShadingEnable = VK_FALSE;
        configInfo.m_MultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        configInfo.m_MultisampleInfo.minSampleShading = 1.0f;
        configInfo.m_MultisampleInfo.pSampleMask = nullptr;
        configInfo.m_MultisampleInfo.alphaToCoverageEnable = VK_FALSE;
        configInfo.m_MultisampleInfo.alphaToOneEnable = VK_FALSE;

        configInfo.m_ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        configInfo.m_ColorBlendAttachment.blendEnable = VK_FALSE;
        configInfo.m_ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        configInfo.m_ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        configInfo.m_ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        configInfo.m_ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        configInfo.m_ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        configInfo.m_ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        configInfo.m_ColorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.m_ColorBlendInfo.logicOpEnable = VK_FALSE;
        configInfo.m_ColorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
        configInfo.m_ColorBlendInfo.attachmentCount = 1;
        configInfo.m_ColorBlendInfo.pAttachments = &configInfo.m_ColorBlendAttachment;
        configInfo.m_ColorBlendInfo.blendConstants[0] = 0.0f;
        configInfo.m_ColorBlendInfo.blendConstants[1] = 0.0f;
        configInfo.m_ColorBlendInfo.blendConstants[2] = 0.0f;
        configInfo.m_ColorBlendInfo.blendConstants[3] = 0.0f;

        configInfo.m_DepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.m_DepthStencilInfo.depthTestEnable = VK_TRUE;
        configInfo.m_DepthStencilInfo.depthWriteEnable = VK_TRUE;
        configInfo.m_DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        configInfo.m_DepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        configInfo.m_DepthStencilInfo.minDepthBounds = 0.0f;
        configInfo.m_DepthStencilInfo.maxDepthBounds = 1.0f;
        configInfo.m_DepthStencilInfo.stencilTestEnable = VK_FALSE;
        configInfo.m_DepthStencilInfo.front = {};
        configInfo.m_DepthStencilInfo.back = {};

        configInfo.m_DynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        configInfo.m_DynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        configInfo.m_DynamicStateInfo.pDynamicStates = configInfo.m_DynamicStateEnables.data();
        configInfo.m_DynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.m_DynamicStateEnables.size());
        configInfo.m_DynamicStateInfo.flags = 0;

        configInfo.m_InputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.m_InputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.m_InputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        configInfo.m_BindingDescriptions = Liara_Model::Vertex::GetBindingDescriptions();
        configInfo.m_AttributeDescriptions = Liara_Model::Vertex::GetAttributeDescriptions();

        // Specialization Constants
        SpecConstant::SpecConstant::Initialize(settings_manager);
        configInfo.m_SpecializationInfo = SpecConstant::SpecConstant::GetSpecializationInfo();
    }

    void Liara_Pipeline::Bind(VkCommandBuffer commandBuffer) const
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
    }

    std::vector<char> Liara_Pipeline::ReadFile(const std::string &filepath)
    {
        const std::string enginePath = ENGINE_DIR + filepath;
        std::ifstream file{ enginePath, std::ios::ate | std::ios::binary };

        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file: " + filepath);
        }

        const size_t fileSize = file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
        file.close();

        return buffer;
    }

    void Liara_Pipeline::CreateGraphicsPipeline(const std::string &vertFilepath, const std::string &fragFilepath, const PipelineConfigInfo &configInfo)
    {
        assert(configInfo.m_PipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no pipelineLayout provided in configInfo");
        assert(configInfo.m_RenderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no renderPass provided in configInfo");

        const auto vertCode = ReadFile(vertFilepath);
        const auto fragCode = ReadFile(fragFilepath);

        CreateShaderModule(vertCode, &m_VertShaderModule);
        CreateShaderModule(fragCode, &m_FragShaderModule);

        VkPipelineShaderStageCreateInfo shaderStages[2];

        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = m_VertShaderModule;
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = &configInfo.m_SpecializationInfo;

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = m_FragShaderModule;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = &configInfo.m_SpecializationInfo;

        const auto& bindingDescriptions = configInfo.m_BindingDescriptions;
        const auto& attributeDescriptions = configInfo.m_AttributeDescriptions;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &configInfo.m_InputAssemblyInfo;
        pipelineInfo.pViewportState = &configInfo.m_ViewportInfo;
        pipelineInfo.pRasterizationState = &configInfo.m_RasterizationInfo;
        pipelineInfo.pMultisampleState = &configInfo.m_MultisampleInfo;
        pipelineInfo.pColorBlendState = &configInfo.m_ColorBlendInfo;
        pipelineInfo.pDepthStencilState = &configInfo.m_DepthStencilInfo;
        pipelineInfo.pDynamicState = &configInfo.m_DynamicStateInfo;

        pipelineInfo.layout = configInfo.m_PipelineLayout;
        pipelineInfo.renderPass = configInfo.m_RenderPass;
        pipelineInfo.subpass = configInfo.m_Subpass;

        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        // FIXME: This is a temporary fix for linux
        // For some reason, m_GraphicsPipeline is not being set correctly,
        // Resulting in a segfault when trying to create the pipeline.
        // This ugly hack is to make sure that the pipeline is created correctly,
        // But a better solution should be found.
        #ifdef __linux__
            const auto new_pipeline  = std::make_unique<VkPipeline>();
            if (vkCreateGraphicsPipelines(m_Device.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, new_pipeline.get()) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create graphics pipeline!");
            }
            m_GraphicsPipeline = *new_pipeline;
        #else
            if (vkCreateGraphicsPipelines(m_Device.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create graphics pipeline!");
            }
        #endif
    }

    void Liara_Pipeline::CreateShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule) const
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        if (vkCreateShaderModule(m_Device.GetDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create shader module!");
        }
    }
}