//
// Created by antoi on 20/10/2024.
//

#pragma once
#include <memory>

#include "../../Liara_Device.h"
#include "../../Liara_GameObject.h"
#include "../../Liara_Renderer.h"
#include "../../Liara_Window.h"

namespace Liara
{
    class SierpinskiApp
    {
    public:
        static constexpr unsigned short WIDTH = 800;
        static constexpr unsigned short HEIGHT = 600;

        SierpinskiApp();
        ~SierpinskiApp();
        SierpinskiApp(const SierpinskiApp&) = delete;
        SierpinskiApp& operator=(const SierpinskiApp&) = delete;

        void Run();

    private:
        void LoadGameObjects();

        void SierpinskiTriangle(std::vector<Liara_Model::Vertex>& vertices, int depth, Liara_Model::Vertex v0, Liara_Model::Vertex v1, Liara_Model::Vertex v2);

        Liara_Window m_Window{ "SierpinskiTriangle With Vulkan", WIDTH, HEIGHT };
        Liara_Device m_Device{ m_Window };
        Liara_Renderer m_Renderer{m_Window, m_Device};
        std::vector<Liara_GameObject> m_GameObjects;
    };
}
