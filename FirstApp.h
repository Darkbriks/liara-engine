//
// Created by antoi on 15/10/2024.
//

#pragma once
#include <memory>

#include "Liara_Window.h"
#include "Liara_Pipeline.h"
#include "Liara_Device.h"
#include "Liara_SwapChain.h"
#include "Liara_Model.h"

namespace Liara
{
    class FirstApp
    {
    public:
        static constexpr unsigned short WIDTH = 800;
        static constexpr unsigned short HEIGHT = 600;

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp&) = delete;
        FirstApp& operator=(const FirstApp&) = delete;

        void Run();

    private:
        void CreatePipeline();
        void CreatePipelineLayout();
        void CreateCommandBuffers();
        void FreeCommandBuffers();
        void DrawFrame();
        void CreateSwapChain();
        void RecordCommandBuffer(uint32_t imageIndex);

        void SierpinskiTriangle(std::vector<Liara_Model::Vertex>& vertices, int depth, Liara_Model::Vertex v0, Liara_Model::Vertex v1, Liara_Model::Vertex v2);
        void LoadModel();

        Liara_Window m_Window{ "Hello Vulkan!", WIDTH, HEIGHT };
        Liara_Device m_Device{ m_Window };
        std::unique_ptr<Liara_SwapChain> m_SwapChain;

        std::unique_ptr<Liara_Pipeline> m_Pipeline;
        VkPipelineLayout m_PipelineLayout;
        std::vector<VkCommandBuffer> m_CommandBuffers;
        std::unique_ptr<Liara_Model> m_Model;
    };
}
