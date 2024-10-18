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

    void Liara_Window::FramebufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        const auto liara_window = static_cast<Liara_Window*>(glfwGetWindowUserPointer(window));
        liara_window->m_resized = true;
        liara_window->m_width = width;
        liara_window->m_height = height;
    }

    void Liara_Window::InitWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        m_Window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(m_Window, this);
        glfwSetFramebufferSizeCallback(m_Window, FramebufferResizeCallback);

    }

}