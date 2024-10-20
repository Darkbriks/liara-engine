//
// Created by antoi on 20/10/2024.
//

#pragma once

#include "../../Liara_Device.h"
#include "../../Liara_GameObject.h"
#include "../../Liara_Renderer.h"
#include "../../Liara_Window.h"

namespace Liara
{
    class GravityVecFieldApp
    {
    public:
        static constexpr unsigned short WIDTH = 800;
        static constexpr unsigned short HEIGHT = 800;

        GravityVecFieldApp() = default;
        ~GravityVecFieldApp() = default;
        GravityVecFieldApp(const GravityVecFieldApp&) = delete;
        GravityVecFieldApp& operator=(const GravityVecFieldApp&) = delete;

        void Run();

    private:
        Liara_Window m_Window{ "GravityVecField With Vulkan", WIDTH, HEIGHT };
        Liara_Device m_Device{ m_Window };
        Liara_Renderer m_Renderer{m_Window, m_Device};
        std::vector<Liara_GameObject> m_GameObjects;
    };
}
