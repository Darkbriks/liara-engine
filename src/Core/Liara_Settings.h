/**
 * @file Liara_Settings.h
 * @brief Defines the `Liara_Settings` class, which encapsulates settings for the Liara engine.
 */

#pragma once

#include "Utils/Singleton.h"

#include <cstdint>
#include <unordered_map>
#include <functional>

#include <vulkan/vulkan_core.h>

namespace Liara
{
    /**
     * @struct WindowSettings
     * @brief Structure that encapsulates settings and flags for a window.
     */
    struct WindowSettings
    {
        uint16_t width = 1280;
        uint16_t height = 720;
        int x_pos = 100;
        int y_pos = 100;
        bool fullscreen = false;
        bool resizable = true;

        // Flags
        bool window_state_change = false;           ///< Whether the window state has changed.
        bool resized = false;                       ///< Whether the window has been resized
        bool fullscreen_change = false;             ///< Whether the window has changed to/from fullscreen

        /**
         * @brief Resets the flags to their default values.
         */
        void ResetFlags()
        {
            window_state_change = false;
            resized = false;
            fullscreen_change = false;
        }
    };

    /**
     * @class Liara_Settings
     * @brief Class that encapsulates settings for the Liara engine.
     *
     * This class can be herited to add more settings to the engine.
     */
    class Liara_Settings : public Singleton<Liara_Settings>
    {
    public:
        /**
         * @brief Destructor.
         */
        ~Liara_Settings() override = default;
        Liara_Settings(const Liara_Settings&) = delete;
        Liara_Settings& operator=(const Liara_Settings&) = delete;


        // Engine settings
        static constexpr auto ENGINE_NAME = "Liara Engine";             ///< The name of the engine
        static constexpr uint8_t MAJOR_ENGINE_VERSION = 0;              ///< The major version of the application
        static constexpr uint8_t MINOR_ENGINE_VERSION = 16;             ///< The minor version of the application
        static constexpr uint8_t PATCH_ENGINE_VERSION = 2;              ///< The patch version of the application
        [[nodiscard]] static uint32_t GetVkEngineVersion() { return VK_MAKE_VERSION(MAJOR_ENGINE_VERSION, MINOR_ENGINE_VERSION, PATCH_ENGINE_VERSION); } ///< The Vulkan version of the engine

        // Application settings
        static constexpr auto APP_NAME = "Liara Application";           ///< The name of the application
        static constexpr uint8_t MAJOR_APP_VERSION = 0;                 ///< The major version of the application
        static constexpr uint8_t MINOR_APP_VERSION = 1;                 ///< The minor version of the application
        static constexpr uint8_t PATCH_APP_VERSION = 0;                 ///< The patch version of the application
        [[nodiscard]] static uint32_t GetVkAppVersion() { return VK_MAKE_VERSION(MAJOR_APP_VERSION, MINOR_APP_VERSION, PATCH_APP_VERSION); } ///< The Vulkan version of the application


        [[nodiscard]] bool NeedsSwapchainRecreation() const { return m_PresentModeChanged; }    ///< Whether any of flags that require swapchain recreation are set

        [[nodiscard]] const WindowSettings& GetWindowSettings(const uint8_t windowID) const { return m_WindowSettings.at(windowID); }   ///< Returns the settings for a window. Assumes the window exists.
        [[nodiscard]] const std::unordered_map<uint8_t, WindowSettings>& GetWindowSettings() const { return m_WindowSettings; }         ///< Returns the settings for all windows
        [[nodiscard]] uint8_t GetWindowCount() const { return m_WindowSettings.size(); }                                                ///< Returns the number of windows. Assumes the window exists.
        [[nodiscard]] uint16_t GetWindowWidth(const uint8_t windowID) const { return m_WindowSettings.at(windowID).width; }             ///< Returns the width of a window. Assumes the window exists.
        [[nodiscard]] uint16_t GetWindowHeight(const uint8_t windowID) const { return m_WindowSettings.at(windowID).height; }           ///< Returns the height of a window. Assumes the window exists.
        [[nodiscard]] int GetWindowXPos(const uint8_t windowID) const { return m_WindowSettings.at(windowID).x_pos; }                   ///< Returns the x position of a window. Assumes the window exists.
        [[nodiscard]] int GetWindowYPos(const uint8_t windowID) const { return m_WindowSettings.at(windowID).y_pos; }                   ///< Returns the y position of a window. Assumes the window exists.
        [[nodiscard]] bool IsWindowFullscreen(const uint8_t windowID) const { return m_WindowSettings.at(windowID).fullscreen; }        ///< Returns whether a window is fullscreen. Assumes the window exists.
        [[nodiscard]] bool IsWindowResizable(const uint8_t windowID) const { return m_WindowSettings.at(windowID).resizable; }          ///< Returns whether a window is resizable. Assumes the window exists.
        [[nodiscard]] bool IsVSync() const { return m_VSync; }                                                                          ///< Whether VSync is enabled
        [[nodiscard]] VkPresentModeKHR GetPreferredPresentMode() const { return m_PreferedPresentMode; }                                ///< The preferred present mode

        [[nodiscard]] uint16_t GetMaxLights() const { return m_MaxLights; }                     ///< The maximum number of lights in the scene

        [[nodiscard]] uint16_t GetMaxTextureSize() const { return m_MaxTextureSize; }           ///< The maximum size of a texture
        [[nodiscard]] bool UseMipmaps() const { return m_UseMipmaps; }                          ///< Whether to use mipmaps for textures
        [[nodiscard]] bool UseAnisotropicFiltering() const { return m_AnisotropicFiltering; }   ///< Whether to use anisotropic filtering for textures
        [[nodiscard]] uint8_t GetMaxAnisotropy() const { return m_MaxAnisotropy; }              ///< The maximum anisotropy level
        [[nodiscard]] bool UseBindlessTextures() const { return m_UseBindlessTextures; }        ///< Whether to use bindless textures
        [[nodiscard]] uint16_t GetMaxTextureSamplers() const { return m_MaxTextureSamplers; }   ///< The maximum number of texture samplers

        // Flags
        [[nodiscard]] bool WasStateChanged(const uint8_t windowID) const { return m_WindowSettings.at(windowID).window_state_change; }      ///< Whether the window state has changed
        [[nodiscard]] bool WasResized(const uint8_t windowID) const { return m_WindowSettings.at(windowID).resized; }                       ///< Whether the window has been resized
        [[nodiscard]] bool WasFullscreenChanged(const uint8_t windowID) const { return m_WindowSettings.at(windowID).fullscreen_change; }   ///< Whether the window has changed to/from fullscreen

        /**
         * @brief Creates a window settings structure for a given window ID if it does not exist.
         * @param windowID The ID of the window.
         */
        void CreateWindow(const uint8_t windowID) { if (m_WindowSettings.find(windowID) == m_WindowSettings.end()) { m_WindowSettings[windowID] = {}; } }
        void SetWindowWidth(uint8_t windowID, uint16_t width);                                                                                          ///< Sets the width of a window
        void SetWindowHeight(uint8_t windowID, uint16_t height);                                                                                        ///< Sets the height of a window
        void SetWindowXPos(const int windowID, const int x_pos) { m_WindowSettings[windowID].x_pos = x_pos; }                                           ///< Sets the x position of a window
        void SetWindowYPos(const int windowID, const int y_pos) { m_WindowSettings[windowID].y_pos = std::max(y_pos, 30); }                         ///< Sets the y position of a window. Y position must be at least 30, to avoid the window title bar being hidden
        void SetWindowFullscreen(uint8_t windowID, bool fullscreen);                                                                                    ///< Sets whether a window is fullscreen
        void SetWindowResizable(const uint8_t windowID, const bool resizable) { m_WindowSettings[windowID].resizable = resizable; }                     ///< Sets whether a window is resizable
        void SetVSync(const bool vsync) { if (m_VSync != vsync) { m_VSync = vsync; m_PresentModeChanged = true; } }                                    ///< Sets whether VSync is enabled
        void SetPreferredPresentMode(const VkPresentModeKHR presentMode) { if (m_PreferedPresentMode != presentMode) { m_PreferedPresentMode = presentMode; m_PresentModeChanged = true; } } ///< Sets the preferred present mode

        void SetMaxTextureSize(const uint16_t maxTextureSize) { m_MaxTextureSize = maxTextureSize; }                         ///< The maximum size of a texture
        void SetUseMipmaps(const bool useMipmaps) { m_UseMipmaps = useMipmaps; }                                             ///< Whether to use mipmaps for textures
        void SetAnisotropicFiltering(const bool anisotropicFiltering) { m_AnisotropicFiltering = anisotropicFiltering; }     ///< Whether to use anisotropic filtering for textures
        void SetMaxAnisotropy(const uint8_t maxAnisotropy) { m_MaxAnisotropy = maxAnisotropy; }                              ///< The maximum anisotropy level
        void SetUseBindlessTextures(const bool useBindlessTextures) { m_UseBindlessTextures = useBindlessTextures; }         ///< Whether to use bindless textures
        void SetMaxTextureSamplers(const uint16_t maxTextureSamplers) { m_MaxTextureSamplers = maxTextureSamplers; }         ///< The maximum number of texture samplers

        void ResetWindowFlags(const uint8_t windowID) { m_WindowSettings[windowID].ResetFlags(); }                                                      ///< Resets the flags for a window
        void SwapchainRecreated() { m_PresentModeChanged = false; }                          ///< Sets the swapchain as recreated by resetting the flags

        /**
         * @brief Loads the settings from a file.
         * @param filename The name of the file to load the settings from.
         * @return true if the settings were successfully loaded.
         */
        virtual bool LoadFromFile(const std::string& filename);

        /**
         * @brief Saves the settings to a file.
         * @param filename The name of the file to save the settings to.
         * @param overwrite Whether to overwrite the file if it already exists.
         * @param create Whether to create the file if it does not exist.
         * @return true if the settings were successfully saved.
         */
        virtual bool SaveToFile(const std::string& filename, bool overwrite, bool create) const;

    private:
        friend class Singleton<Liara_Settings>;

        Liara_Settings() = default;                ///< Default constructor

        // Flags
        bool m_PresentModeChanged = false;         ///< Whether the present mode has changed, requiring a swapchain recreation

        // Window settings
        std::unordered_map<uint8_t, WindowSettings> m_WindowSettings = {};      ///< The settings for each window
        bool m_VSync = true;                                                    ///< Whether VSync is enabled. If VSync is enabled, the PreferredPresentMode is ignored
        /**
         * The preferred present mode.
         * Most common values:
         *      - VK_PRESENT_MODE_IMMEDIATE_KHR: The image is transferred to the screen immediately, which may result in tearing, but lower latency.
         *      - VK_PRESENT_MODE_FIFO_KHR: Uses a FIFO queue to present images. The display is synchronized with the vertical blanking period. No tearing, but higher latency.
         *      - VK_PRESENT_MODE_MAILBOX_KHR: Uses a mailbox queue to present images. The display is synchronized with the vertical blanking period. No tearing, lower latency than FIFO, but can need more resources.
         */
        VkPresentModeKHR m_PreferedPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;

        // Graphics settings
        uint16_t m_MaxLights = 10;                 ///< The maximum number of lights in the scene
        // TODO : Anti-aliasing

        // Textures settings
        uint16_t m_MaxTextureSize = 2048;          ///< The maximum size of a texture
        bool m_UseMipmaps = true;                  ///< Whether to use mipmaps for textures
        bool m_AnisotropicFiltering = true;        ///< Whether to use anisotropic filtering for textures
        uint16_t m_MaxAnisotropy = 16;             ///< The maximum anisotropy level
        bool m_UseBindlessTextures = false;         ///< Whether to use bindless textures
        uint16_t m_MaxTextureSamplers = 16;        ///< The maximum number of texture samplers // TODO : Implement

        /**
         * @brief Loads the settings for a window from a file.
         * @param file The file to load the settings from.
         * @param windowID The ID of the window.
         */
        void LoadWindowSettings(std::ifstream& file, uint8_t windowID);
        bool WriteToFile(const std::string& filename) const;              ///< Writes the settings to a file.

        static void SetString(std::string& key, const std::string& value); ///< Sets a string value
        static void SetBool(bool& key, const std::string& value);          ///< Sets a boolean value
        static void SetInt(int& key, const std::string& value);            ///< Sets an integer value
        static void SetUint8(uint8_t& key, const std::string& value);      ///< Sets an unsigned 8-bit integer value
        static void SetUint16(uint16_t& key, const std::string& value);    ///< Sets an unsigned 16-bit integer value
        static void SetUint32(uint32_t& key, const std::string& value);    ///< Sets an unsigned 32-bit integer value

        std::unordered_map<const char*, std::function<void(std::string)>> m_Setters{    ///< Map of setters for the settings
            {"vsync", [this](const std::string& value) { SetBool(m_VSync, value); }},
            {"preferred_present_mode", [this](const std::string& value) { SetUint32(reinterpret_cast<uint32_t&>(m_PreferedPresentMode), value); }},
            {"max_lights", [this](const std::string& value) { SetUint16(m_MaxLights, value); }},
            {"max_texture_size", [this](const std::string& value) { SetUint16(m_MaxTextureSize, value); }},
            {"use_mipmaps", [this](const std::string& value) { SetBool(m_UseMipmaps, value); }},
            {"use_anisotropic_filtering", [this](const std::string& value) { SetBool(m_AnisotropicFiltering, value); }},
            {"max_anisotropy", [this](const std::string& value) { SetUint16(m_MaxAnisotropy, value); }},
            {"use_bindless_textures", [this](const std::string& value) { SetBool(m_UseBindlessTextures, value); }},
            {"max_texture_samplers", [this](const std::string& value) { SetUint16(m_MaxTextureSamplers, value); }},
        };
    };
}