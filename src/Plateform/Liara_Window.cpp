#include "Liara_Window.h"

#include <stdexcept>
#include <utility>

namespace Liara::Plateform
{
    Liara_Window::Liara_Window(std::string  title, const unsigned short width, const unsigned short height) : m_title(std::move(title)), m_width(width), m_height(height)
    {
        InitWindow();
    }

    Liara_Window::~Liara_Window()
    {
        SDL_DestroyWindow(m_Window);
        SDL_Quit();
    }

    void Liara_Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface) const
    {
        if (SDL_Vulkan_CreateSurface(m_Window, instance, surface) != SDL_TRUE)
        {
            throw std::runtime_error("Failed to create window surface!");
        }
    }

    void Liara_Window::FramebufferResizeCallback(SDL_Window *window, const int width, const int height)
    {
        const auto liara_window = static_cast<Liara_Window*>(SDL_GetWindowData(window, "Liara_Window"));
        liara_window->m_resized = true;
        liara_window->m_width = width;
        liara_window->m_height = height;
    }

    void Liara_Window::InitWindow()
    {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) { throw std::runtime_error("Failed to initialize SDL"); }

        constexpr uint32_t window_flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;

        if (!((m_Window = SDL_CreateWindow(m_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_width, m_height, window_flags))))
        {
            throw std::runtime_error("Failed to create window! SDL_Error: " + std::string(SDL_GetError()));
        }

        SDL_SetWindowData(m_Window, "Liara_Window", this);

        SDL_AddEventWatch([](void *userdata, SDL_Event *event) { return static_cast<Liara_Window *>(userdata)->EventCallback(userdata, event); }, this);
    }

    int Liara_Window::EventCallback(void* userdata, const SDL_Event* event)
    {
        if (event->type == SDL_WINDOWEVENT)
        {
            auto *window = SDL_GetWindowFromID(event->window.windowID);
            //auto *lira_window = static_cast<Liara_Window *>(SDL_GetWindowData(window, "Liara_Window"));
            if (event->window.event == SDL_WINDOWEVENT_RESIZED)
            {
                FramebufferResizeCallback(window, event->window.data1, event->window.data2);
            }
        }
        return 0;
    }
}