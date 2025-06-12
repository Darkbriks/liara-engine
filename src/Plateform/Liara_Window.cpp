#include "Liara_Window.h"

#include <stdexcept>

#include <nlohmann/json.hpp>

namespace Liara::Plateform
{
    uint8_t Liara_Window::g_WindowCount = 0;
    std::unordered_map<uint8_t, Liara_Window*> Liara_Window::g_Windows;

    std::string WindowSettings::serialize() const {
        fmt::print("Serializing window settings: name={}, width={}, height={}, xPos={}, yPos={}, fullscreen={}, resizable={}\n",
            name, width, height, xPos, yPos, fullscreen, resizable);
        nlohmann::json j;
        j["name"] = name.data();
        j["width"] = width;
        j["height"] = height;
        j["xPos"] = xPos;
        j["yPos"] = yPos;
        j["fullscreen"] = fullscreen;
        j["resizable"] = resizable;
        return j.dump();
    }

    bool WindowSettings::deserialize(const std::string_view data) {
        try {
            const nlohmann::json j = nlohmann::json::parse(data);
            name = std::string_view(j.value("name", "window.default"));
            width = j.value("width", 1280);
            height = j.value("height", 720);
            xPos = j.value("xPos", 100);
            yPos = j.value("yPos", 100);
            fullscreen = j.value("fullscreen", false);
            resizable = j.value("resizable", true);
            fmt::print("Deserialized window settings: name={}, width={}, height={}, xPos={}, yPos={}, fullscreen={}, resizable={}\n",
                name, width, height, xPos, yPos, fullscreen, resizable);
            return true;
        } catch (const nlohmann::json::parse_error& e) {
            fmt::print(stderr, "Failed to deserialize window settings: {}\n", e.what());
            return false;
        }
    }

    Liara_Window::Liara_Window(Core::Liara_SettingsManager& settingsManager) : m_SettingsManager(settingsManager), m_ID(g_WindowCount++)
    {
        g_Windows[m_ID] = this;
        WindowSettings settings;
        settings.name = std::string_view("Liara_Window " + std::to_string(m_ID));
        m_SettingsManager.RegisterSetting("window." + std::to_string(m_ID), settings, Core::SettingFlags::Default, true);
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
        const auto settings = m_SettingsManager.Get<WindowSettings>("window." + std::to_string(m_ID));
        return {
            static_cast<uint32_t>(settings.width),
            static_cast<uint32_t>(settings.height)
        };
    }

    void Liara_Window::ResizeWindow() const
    {
        auto [width, height] = GetExtent();
        SDL_SetWindowSize(
            m_Window,
            static_cast<int>(width),
            static_cast<int>(height)
            );
    }

    void Liara_Window::UpdateFullscreenMode() const
    {
        if (const auto settings = m_SettingsManager.Get<WindowSettings>("window." + std::to_string(m_ID));
            settings.fullscreen)
        {
            SDL_SetWindowFullscreen(m_Window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
        else
        {
            SDL_SetWindowFullscreen(m_Window, 0);
            ResizeWindow();
        }
    }

    void Liara_Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface) const
    {
        if (SDL_Vulkan_CreateSurface(m_Window, instance, surface) != SDL_TRUE)
        {
            throw std::runtime_error("Failed to create window surface!");
        }
    }

    void Liara_Window::FramebufferResizeCallback(const uint8_t windowID, const int width, const int height) const
    {
        auto settings = m_SettingsManager.Get<WindowSettings>("window." + std::to_string(windowID));
        settings.SetWidth(width);
        settings.SetHeight(height);
        m_SettingsManager.Set("window." + std::to_string(windowID), settings);
    }

    void Liara_Window::InitWindow()
    {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) { throw std::runtime_error("Failed to initialize SDL"); }

        const auto settings = m_SettingsManager.Get<WindowSettings>("window." + std::to_string(m_ID));

        uint32_t window_flags = SDL_WINDOW_VULKAN;
        if (settings.resizable) { window_flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE; }

        if (!((m_Window = SDL_CreateWindow(settings.name.data(), settings.GetXPos(), settings.GetYPos(), settings.GetWidth(), settings.GetHeight(), window_flags))))
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
                auto settings = lira_window->m_SettingsManager.Get<WindowSettings>("window." + std::to_string(lira_window->GetID()));
                settings.SetXPos(event->window.data1);
                settings.SetYPos(event->window.data2);
                lira_window->m_SettingsManager.Set("window." + std::to_string(lira_window->GetID()), settings);
            }
            if (event->window.event == SDL_WINDOW_MINIMIZED) { m_minimized = true; }
            if (event->window.event == SDL_WINDOWEVENT_RESTORED) { m_minimized = false; }
            if (event->window.event == SDL_WINDOWEVENT_CLOSE) { m_quit_requested = true; }
        }
        return 0;
    }
}