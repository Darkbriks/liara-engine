//
// Created by antoi on 15/10/2024.
//

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace Liara
{
    class Liara_Window
    {
    public:
        Liara_Window(std::string  title, unsigned short width, unsigned short height);
        ~Liara_Window();

        Liara_Window(const Liara_Window&) = delete;
        Liara_Window& operator=(const Liara_Window&) = delete;

        [[nodiscard]] bool ShouldClose() const { return glfwWindowShouldClose(m_Window); }
        [[nodiscard]] VkExtent2D GetExtent() const { return { m_width, m_height }; }

        void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface) const;

    private:
        GLFWwindow* m_Window;

        std::string m_title;
        const unsigned short m_width;
        const unsigned short m_height;

        void InitWindow();
    };
}