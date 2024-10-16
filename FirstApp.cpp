//
// Created by antoi on 15/10/2024.
//

#include "FirstApp.h"

namespace Liara
{
    void FirstApp::Run()
    {
        while (!m_Window.ShouldClose())
        {
            glfwPollEvents();
        }
    }
}