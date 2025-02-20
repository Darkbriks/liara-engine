#include "Liara_Window.h"
#include "Core/Liara_Settings.h"

#include <stdexcept>

namespace Liara::Plateform
{
    uint8_t Liara_Window::g_WindowCount = 0;
    std::unordered_map<uint8_t, Liara_Window*> Liara_Window::g_Windows;

    Liara_Window::Liara_Window() : m_ID(g_WindowCount++)
    {
        g_Windows[m_ID] = this;
        Singleton<Liara_Settings>::GetInstanceSync().CreateWindow(m_ID);
        InitWindow();
    }

    Liara_Window::~Liara_Window()
    {
        g_Windows.erase(m_ID);
        SDL_DestroyWindow(m_Window);
        SDL_Quit();
    }

    VkExtent2D Liara_Window::GetExtent() const
    {
        return{
            Singleton<Liara_Settings>::GetInstanceSync().GetWindowWidth(m_ID),
            Singleton<Liara_Settings>::GetInstanceSync().GetWindowHeight(m_ID)
        };
    }

    void Liara_Window::ResizeWindow() const
    {
        SDL_SetWindowSize(
            m_Window,
            Singleton<Liara_Settings>::GetInstanceSync().GetWindowWidth(m_ID),
            Singleton<Liara_Settings>::GetInstanceSync().GetWindowHeight(m_ID)
            );
    }

    void Liara_Window::UpdateFullscreenMode() const
    {
        SDL_SetWindowFullscreen(m_Window, Singleton<Liara_Settings>::GetInstance().IsWindowFullscreen(m_ID) ? SDL_WINDOW_FULLSCREEN : 0);
    }

    void Liara_Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface) const
    {
        if (SDL_Vulkan_CreateSurface(m_Window, instance, surface) != SDL_TRUE)
        {
            throw std::runtime_error("Failed to create window surface!");
        }
    }

    void Liara_Window::FramebufferResizeCallback(const uint8_t windowID, const int width, const int height)
    {
        Singleton<Liara_Settings>::GetInstanceSync().SetWindowWidth(windowID, width);
        Singleton<Liara_Settings>::GetInstanceSync().SetWindowHeight(windowID, height);
    }

    void Liara_Window::InitWindow()
    {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) { throw std::runtime_error("Failed to initialize SDL"); }

        const Liara_Settings& settings = Singleton<Liara_Settings>::GetInstanceSync();

        uint32_t window_flags = SDL_WINDOW_VULKAN;
        if (settings.IsWindowResizable(m_ID)) { window_flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE; }

        if (!((m_Window = SDL_CreateWindow(settings.GetAppName().c_str(), settings.GetWindowXPos(m_ID), settings.GetWindowYPos(m_ID), settings.GetWindowWidth(m_ID), settings.GetWindowHeight(m_ID), window_flags))))
        {
            throw std::runtime_error("Failed to create window! SDL_Error: " + std::string(SDL_GetError()));
        }

        UpdateFullscreenMode();

        SDL_SetWindowData(m_Window, "Liara_Window", this);
        SDL_AddEventWatch([](void *userdata, SDL_Event *event) { return static_cast<Liara_Window *>(userdata)->EventCallback(userdata, event); }, this);
    }

    int Liara_Window::EventCallback(void* userdata, const SDL_Event* event)
    {
        if (event->type == SDL_WINDOWEVENT)
        {
            auto *window = SDL_GetWindowFromID(event->window.windowID);
            const auto *lira_window = static_cast<Liara_Window *>(SDL_GetWindowData(window, "Liara_Window"));
            if (event->window.event == SDL_WINDOWEVENT_RESIZED)
            {
                FramebufferResizeCallback(lira_window->GetID(), event->window.data1, event->window.data2);
            }
            if (event->window.event == SDL_WINDOWEVENT_MOVED)
            {
                Singleton<Liara_Settings>::GetInstanceSync().SetWindowXPos(lira_window->GetID(), event->window.data1);
                Singleton<Liara_Settings>::GetInstanceSync().SetWindowYPos(lira_window->GetID(), event->window.data2);
            }
            if (event->window.event == SDL_WINDOW_MINIMIZED) { m_minimized = true; }
            if (event->window.event == SDL_WINDOWEVENT_RESTORED) { m_minimized = false; }
            if (event->window.event == SDL_WINDOWEVENT_CLOSE) { m_quit_requested = true; }
        }
        return 0;
    }
}