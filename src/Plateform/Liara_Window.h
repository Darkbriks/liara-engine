#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
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

        [[nodiscard]] bool ShouldClose() const { return m_quit_requested; }
        [[nodiscard]] VkExtent2D GetExtent() const { return { m_width, m_height }; }
        [[nodiscard]] bool WasResized() const { return m_resized; }
        [[nodiscard]] bool WasMinimized() const { return m_minimized; }
        [[nodiscard]] SDL_Window* GetWindow() const { return m_Window; }

        void ResetResizedFlag() { m_resized = false; }

        void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface) const;

    private:
        static void FramebufferResizeCallback(SDL_Window* window, int width, int height);
        SDL_Window* m_Window{};

        std::string m_title;
        unsigned short m_width;
        unsigned short m_height;
        bool m_resized = false;
        bool m_minimized = false;
        bool m_quit_requested = false;

        void InitWindow();
        int EventCallback(void *userdata, const SDL_Event *event);
    };
}