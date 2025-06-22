/**
 * @file Liara_Window.h
 * @brief Defines the `Liara_Window` class, which encapsulates a SDL2 window.
 *
 * For now, window must be unique, but I plan to add support for multiple windows.
 */

#pragma once

#include "Core/Liara_SettingSerializer.h"
#include "Core/Liara_SettingsManager.h"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <string>
#include <string_view>
#include <unordered_map>

namespace Liara::Plateform
{
    class WindowSettings final : public Core::ISettingSerializable
    {
    public:
        std::string_view name = "window.default";
        uint16_t width = 1280, height = 720;
        int xPos = 50, yPos = 50;
        bool fullscreen = false, resizable = true;
        bool wasResized = false, wasFullscreenChanged = false;


        [[nodiscard]] uint16_t GetWidth() const { return width; }
        [[nodiscard]] uint16_t GetHeight() const { return height; }
        [[nodiscard]] int GetXPos() const { return xPos; }
        [[nodiscard]] int GetYPos() const { return yPos; }
        [[nodiscard]] bool IsFullscreen() const { return fullscreen; }
        [[nodiscard]] bool IsResizable() const { return resizable; }

        void SetWidth(const uint16_t newWidth) {
            width = newWidth;
            wasResized = true;
        }
        void SetHeight(const uint16_t newHeight) {
            height = newHeight;
            wasResized = true;
        }
        void SetXPos(const int newXPos) { xPos = newXPos; }
        void SetYPos(const int newYPos) { yPos = newYPos; }
        void SetFullscreen(const bool newFullscreen) {
            if (fullscreen != newFullscreen) {
                fullscreen = newFullscreen;
                wasFullscreenChanged = true;
            }
        }

        void ResetFlags() { wasResized = wasFullscreenChanged = false; }

        [[nodiscard]] std::string serialize() const override;
        bool deserialize(std::string_view data) override;
    };

    /**
     * @class Liara_Window
     * @brief Class that encapsulates a SDL2 window.
     */
    class Liara_Window
    {
    public:
        /**
         * @brief Constructor, initializes id and calls InitWindow.
         * @param settingsManager Reference to the settings manager to use for window settings.
         */
        explicit Liara_Window(Core::Liara_SettingsManager& settingsManager);

        /**
         * @brief Destructor, destroys the window and quits SDL.
         */
        ~Liara_Window();

        Liara_Window(const Liara_Window&) = delete;
        Liara_Window& operator=(const Liara_Window&) = delete;

        static Liara_Window* GetWindow(const uint8_t id) {
            return windows[id];
        }  ///< Returns the window with the given ID

        [[nodiscard]] bool ShouldClose() const { return m_quit_requested; }  ///< Whether the window should close
        [[nodiscard]] VkExtent2D GetExtent() const;                          ///< Returns the window extent
        [[nodiscard]] bool WasMinimized() const { return m_minimized; }      ///< Whether the window was minimized
        [[nodiscard]] SDL_Window* GetWindow() const { return m_Window; }     ///< Returns the SDL window
        [[nodiscard]] uint8_t GetID() const { return m_ID; }                 ///< Returns the window ID

        void CreateWindowSurface(VkInstance instance,
                                 VkSurfaceKHR* surface) const;  ///< Creates a Vulkan surface for the window

        void ResizeWindow() const;          ///< Resizes the window
        void UpdateFullscreenMode() const;  ///< Updates the fullscreen mode

    private:
        /**
         * @brief Callback function for the framebuffer resize event.
         * @param windowID The unique ID of the window.
         * @param width The new width of the window.
         * @param height The new height of the window.
         */
        void FramebufferResizeCallback(uint8_t windowID, int width, int height) const;

        Core::Liara_SettingsManager& m_SettingsManager;

        static uint8_t windowCount;                                 ///< The number of windows created
        static std::unordered_map<uint8_t, Liara_Window*> windows;   ///< The map of windows

        SDL_Window* m_Window{};  ///< The SDL window
        const uint8_t m_ID;      ///< The unique ID of the window, for a hypothetical multiple windows support

        bool m_minimized = false;       ///< Whether the window is minimized
        bool m_quit_requested = false;  ///< Whether the window should close

        void InitWindow();  ///< Initializes the window

        /**
         * @brief Event callback function.
         * @param userdata The user data.
         * @param event The SDL event.
         * @return 1 if the event was handled, 0 otherwise.
         */
        int EventCallback(void* userdata, const SDL_Event* event);
    };
}