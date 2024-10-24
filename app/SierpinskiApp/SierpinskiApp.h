//
// Created by antoi on 20/10/2024.
//

#pragma once
#include <memory>

#include "Graphics/Liara_Device.h"
#include "Graphics/Liara_Renderer.h"
#include "Core/Liara_GameObject.h"
#include "Plateform/Liara_Window.h"

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

    void SierpinskiTriangle(std::vector<Liara::Graphics::Liara_Model::Vertex>& vertices, int depth, Liara::Graphics::Liara_Model::Vertex v0, Liara::Graphics::Liara_Model::Vertex v1, Liara::Graphics::Liara_Model::Vertex v2);

    Liara::Plateform::Liara_Window m_Window{ "SierpinskiTriangle With Vulkan", WIDTH, HEIGHT };
    Liara::Graphics::Liara_Device m_Device{ m_Window };
    Liara::Graphics::Liara_Renderer m_Renderer{m_Window, m_Device};
    std::vector<Liara::Core::Liara_GameObject> m_GameObjects;
};
