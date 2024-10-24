//
// Created by antoi on 15/10/2024.
//

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <string>

namespace Liara::Plateform
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
        [[nodiscard]] bool WasResized() const { return m_resized; }
        [[nodiscard]] GLFWwindow* GetWindow() const { return m_Window; }

        void ResetResizedFlag() { m_resized = false; }

        void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface) const;

    private:
        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
        GLFWwindow* m_Window;

        std::string m_title;
        unsigned short m_width;
        unsigned short m_height;
        bool m_resized = false;

        void InitWindow();
    };
}