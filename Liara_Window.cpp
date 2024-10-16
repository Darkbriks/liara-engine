//
// Created by antoi on 15/10/2024.
//

#include "Liara_Window.h"

#include <stdexcept>
#include <utility>

namespace Liara
{
    Liara_Window::Liara_Window(std::string  title, const unsigned short width, const unsigned short height) : m_title(std::move(title)), m_width(width), m_height(height)
    {
        InitWindow();
    }

    Liara_Window::~Liara_Window()
    {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    void Liara_Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface) const
    {
        if (glfwCreateWindowSurface(instance, m_Window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create window surface!");
        }
    }


    void Liara_Window::InitWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        m_Window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);

    }

}