//
// Created by antoi on 15/10/2024.
//

#pragma once
#include "Liara_Window.h"
#include "Liara_Pipeline.h"
#include "Liara_Device.h"

namespace Liara {
    class FirstApp {
    public:
        static constexpr unsigned short WIDTH = 800;
        static constexpr unsigned short HEIGHT = 600;

        void Run();

    private:
        Liara_Window m_Window{ "Hello Vulkan!", WIDTH, HEIGHT };
        Liara_Device m_Device{ m_Window };
        Liara_Pipeline m_Pipeline{m_Device, "shaders/SimpleShader.vert.spv", "shaders/SimpleShader.frag.spv", Liara_Pipeline::DefaultPipelineConfigInfo(WIDTH, HEIGHT) };
    };
}
