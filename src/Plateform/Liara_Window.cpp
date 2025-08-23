#include "Liara_Window.h"

#include "Core/Liara_SettingsManager.h"
#include "Core/Logging/LogMacros.h"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <nlohmann/json.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace Liara::Plateform
{
    uint8_t Liara_Window::windowCount = 0;
    std::unordered_map<uint8_t, Liara_Window*> Liara_Window::windows;

    std::string WindowSettings::serialize() const {
        LIARA_LOG_DEBUG(LogPlatform,
                        "Serializing window settings: name={}, width={}, height={}, xPos={}, yPos={}, fullscreen={}, "
                        "resizable={}",
                        name,
                        width,
                        height,
                        xPos,
                        yPos,
                        fullscreen,
                        resizable);
        nlohmann::json json;
        json["name"] = name.data();
        json["width"] = width;
        json["height"] = height;
        json["xPos"] = xPos;
        json["yPos"] = yPos;
        json["fullscreen"] = fullscreen;
        json["resizable"] = resizable;
        return json.dump();
    }

    bool WindowSettings::deserialize(const std::string_view data) {
        try {
            const nlohmann::json json = nlohmann::json::parse(data);
            name = std::string_view(json.value("name", "window.default"));
            width = static_cast<uint16_t>(json.value("width", 1280));
            height = static_cast<uint16_t>(json.value("height", 720));
            xPos = json.value("xPos", 100);
            yPos = json.value("yPos", 100);
            fullscreen = json.value("fullscreen", false);
            resizable = json.value("resizable", true);
            LIARA_LOG_DEBUG(LogPlatform,
                            "Deserialized window settings: name={}, width={}, height={}, xPos={}, yPos={}, "
                            "fullscreen={}, resizable={}",
                            name,
                            width,
                            height,
                            xPos,
                            yPos,
                            fullscreen,
                            resizable);
            return true;
        }
        catch (const nlohmann::json::parse_error& e) {
            LIARA_LOG_ERROR(LogPlatform, "Failed to deserialize window settings: {}", e.what());
            return false;
        }
    }

    Liara_Window::Liara_Window(Core::Liara_SettingsManager& settingsManager)
        : m_SettingsManager(settingsManager)
        , m_ID(windowCount++) {
        windows[m_ID] = this;
        WindowSettings settings;
        settings.name = std::string_view("Liara_Window " + std::to_string(m_ID));
        m_SettingsManager.RegisterSetting(
            "window." + std::to_string(m_ID), settings, Core::SettingFlags::DEFAULT, true);
        InitWindow();
    }

    Liara_Window::~Liara_Window() {
        windows.erase(m_ID);
        SDL_DestroyWindow(m_Window);
        SDL_Quit();
    }

    VkExtent2D Liara_Window::GetExtent() const {
        const auto settings = m_SettingsManager.Get<WindowSettings>("window." + std::to_string(m_ID));
        return {static_cast<uint32_t>(settings.width), static_cast<uint32_t>(settings.height)};
    }

    void Liara_Window::ResizeWindow() const {
        auto [width, height] = GetExtent();
        SDL_SetWindowSize(m_Window, static_cast<int>(width), static_cast<int>(height));
    }

    void Liara_Window::UpdateFullscreenMode() const {
        if (const auto settings = m_SettingsManager.Get<WindowSettings>("window." + std::to_string(m_ID));
            settings.fullscreen) {
            SDL_SetWindowFullscreen(m_Window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
        else {
            SDL_SetWindowFullscreen(m_Window, 0);
            ResizeWindow();
        }
    }

    void Liara_Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface) const {
        if (SDL_Vulkan_CreateSurface(m_Window, instance, surface) != SDL_TRUE) {
            LIARA_THROW_RUNTIME_ERROR(LogPlatform, "Failed to create window surface! SDL_Error: {}", SDL_GetError());
        }
    }

    void Liara_Window::FramebufferResizeCallback(const uint8_t windowID, const int width, const int height) const {
        auto settings = m_SettingsManager.Get<WindowSettings>("window." + std::to_string(windowID));
        settings.SetWidth(static_cast<uint16_t>(width));
        settings.SetHeight(static_cast<uint16_t>(height));
        m_SettingsManager.Set("window." + std::to_string(windowID), settings);
    }

    void Liara_Window::InitWindow() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            LIARA_THROW_RUNTIME_ERROR(LogPlatform, "Failed to initialize SDL! SDL_Error: {}", SDL_GetError());
        }

        const auto settings = m_SettingsManager.Get<WindowSettings>("window." + std::to_string(m_ID));

        uint32_t windowFlags = SDL_WINDOW_VULKAN;
        if (settings.resizable) { windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE; }

        if ((m_Window = SDL_CreateWindow(settings.name.data(),
                                         settings.GetXPos(),
                                         settings.GetYPos(),
                                         settings.GetWidth(),
                                         settings.GetHeight(),
                                         windowFlags))
            == nullptr) {
            LIARA_THROW_RUNTIME_ERROR(LogPlatform, "Failed to create SDL window! SDL_Error: {}", SDL_GetError());
        }

        UpdateFullscreenMode();

        SDL_SetWindowData(m_Window, "Liara_Window", this);
        SDL_AddEventWatch(
            [](void* userdata, SDL_Event* event) {
                return static_cast<Liara_Window*>(userdata)->EventCallback(userdata, event);
            },
            this);
    }

    int Liara_Window::EventCallback(void* /*userdata*/, const SDL_Event* event) {
        if (event->type == SDL_WINDOWEVENT) {
            auto* window = SDL_GetWindowFromID(event->window.windowID);
            const auto* liraWindow = static_cast<Liara_Window*>(SDL_GetWindowData(window, "Liara_Window"));
            if (event->window.event == SDL_WINDOWEVENT_RESIZED) {
                FramebufferResizeCallback(liraWindow->GetID(), event->window.data1, event->window.data2);
            }
            if (event->window.event == SDL_WINDOWEVENT_MOVED) {
                auto settings =
                    liraWindow->m_SettingsManager.Get<WindowSettings>("window." + std::to_string(liraWindow->GetID()));
                settings.SetXPos(event->window.data1);
                settings.SetYPos(event->window.data2);
                liraWindow->m_SettingsManager.Set("window." + std::to_string(liraWindow->GetID()), settings);
            }
            if (event->window.event == SDL_WINDOW_MINIMIZED) { m_minimized = true; }
            if (event->window.event == SDL_WINDOWEVENT_RESTORED) { m_minimized = false; }
            if (event->window.event == SDL_WINDOWEVENT_CLOSE) { m_quit_requested = true; }
        }
        return 0;
    }
}