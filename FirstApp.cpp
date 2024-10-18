//
// Created by antoi on 15/10/2024.
//

#include "FirstApp.h"

#include <array>
#include <stdexcept>

namespace Liara
{
    FirstApp::FirstApp()
    {
        LoadModel();
        CreatePipelineLayout();
        CreatePipeline();
        CreateCommandBuffers();
    }

    FirstApp::~FirstApp()
    {
        vkDestroyPipelineLayout(m_Device.GetDevice(), m_PipelineLayout, nullptr);
    }

    void FirstApp::Run()
    {
        while (!m_Window.ShouldClose())
        {
            glfwPollEvents();
            DrawFrame();
        }

        vkDeviceWaitIdle(m_Device.GetDevice());
    }

    void FirstApp::CreatePipeline()
    {
        auto pipelineConfig = Liara_Pipeline::DefaultPipelineConfigInfo(m_SwapChain.Width(), m_SwapChain.Height());
        pipelineConfig.m_RenderPass = m_SwapChain.GetRenderPass();
        pipelineConfig.m_PipelineLayout = m_PipelineLayout;
        m_Pipeline = std::make_unique<Liara_Pipeline>(m_Device, "shaders/SimpleShader.vert.spv", "shaders/SimpleShader.frag.spv", pipelineConfig);
    }

    void FirstApp::CreatePipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create pipeline layout!");
        }
    }

    void FirstApp::CreateCommandBuffers()
    {
        m_CommandBuffers.resize(m_SwapChain.ImageCount());

        VkCommandBufferAllocateInfo commandBufferAllocInfo{};
        commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocInfo.commandPool = m_Device.GetCommandPool();
        commandBufferAllocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

        if (vkAllocateCommandBuffers(m_Device.GetDevice(), &commandBufferAllocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate command buffers!");
        }

        for (size_t i = 0; i < m_CommandBuffers.size(); i++)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(m_CommandBuffers[i], &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = m_SwapChain.GetRenderPass();
            renderPassInfo.framebuffer = m_SwapChain.GetFrameBuffer(static_cast<int>(i));
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = m_SwapChain.GetSwapChainExtent();

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};

            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            m_Pipeline->Bind(m_CommandBuffers[i]);
            m_Model->Bind(m_CommandBuffers[i]);
            m_Model->Draw(m_CommandBuffers[i]);

            vkCmdEndRenderPass(m_CommandBuffers[i]);
            if (vkEndCommandBuffer(m_CommandBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to record command buffer!");
            }
        }
    }

    void FirstApp::DrawFrame()
    {
        uint32_t imageIndex;
        auto result = m_SwapChain.AcquireNextImage(&imageIndex);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to acquire next image!");
        }

        result = m_SwapChain.SubmitCommandBuffers(&m_CommandBuffers[imageIndex], &imageIndex);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to submit command buffer!");
        }
    }

    void FirstApp::SierpinskiTriangle(std::vector<Liara_Model::Vertex> &vertices, int depth, Liara_Model::Vertex v0, Liara_Model::Vertex v1, Liara_Model::Vertex v2)
    {
        if (depth <= 0)
        {
            vertices.push_back({v0});
            vertices.push_back({v1});
            vertices.push_back({v2});
        }
        else
        {
            // Calculate midpoints of sides, and interpolate color
            Liara_Model::Vertex v01 = {(v0.position + v1.position) / 2.0f, (v0.color + v1.color) / 2.0f};
            Liara_Model::Vertex v12 = {(v1.position + v2.position) / 2.0f, (v1.color + v2.color) / 2.0f};
            Liara_Model::Vertex v20 = {(v2.position + v0.position) / 2.0f, (v2.color + v0.color) / 2.0f};

            SierpinskiTriangle(vertices, depth - 1, v0, v01, v20);
            SierpinskiTriangle(vertices, depth - 1, v01, v1, v12);
            SierpinskiTriangle(vertices, depth - 1, v20, v12, v2);
        }
    }


    void FirstApp::LoadModel()
    {
        /*std::vector<Liara_Model::Vertex> vertices{
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };*/

        std::vector<Liara_Model::Vertex> vertices;
        SierpinskiTriangle(vertices, 6, {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}}, {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}});
        m_Model = std::make_unique<Liara_Model>(m_Device, vertices);
    }

}
