//
// Created by antoi on 15/10/2024.
//

#pragma once

#include "../../Liara_Device.h"
#include "../../Liara_GameObject.h"
#include "../../Liara_Renderer.h"
#include "../../Liara_Window.h"

namespace Liara
{
    class FirstApp
    {
    public:
        static constexpr unsigned short WIDTH = 800;
        static constexpr unsigned short HEIGHT = 600;

        FirstApp();
        ~FirstApp() = default;
        FirstApp(const FirstApp&) = delete;
        FirstApp& operator=(const FirstApp&) = delete;

        void Run();

    private:
        void LoadGameObjects();

        Liara_Window m_Window{ "Test App With Vulkan", WIDTH, HEIGHT };
        Liara_Device m_Device{ m_Window };
        Liara_Renderer m_Renderer{m_Window, m_Device};
        std::vector<Liara_GameObject> m_GameObjects;
    };
}
