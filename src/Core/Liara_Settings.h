/**
 * @file Liara_Settings.h
 * @brief Defines the `Liara_Settings` class, which encapsulates settings for the Liara engine.
 */

#pragma once

#include "Utils/Singleton.h"

#include <cstdint>
#include <string>
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
        virtual ~Liara_Settings() = default;
        Liara_Settings(const Liara_Settings&) = delete;
        Liara_Settings& operator=(const Liara_Settings&) = delete;

        /**
         * @brief Returns a flag indicating whether the pipeline has been loaded.
         *
         * If the pipeline has been loaded, the settings should not be modified.
         *
         * @return true if the pipeline has been loaded.
         */
        [[nodiscard]] bool IsPipelineLoaded() const { return m_PipelineLoaded; }

        [[nodiscard]] std::string GetEngineName() const { return m_EngineName; }             ///< The name of the engine
        [[nodiscard]] uint8_t GetEngineVersionMajor() const { return m_MajorVersion; }          ///< The major version of the engine
        [[nodiscard]] uint8_t GetEngineVersionMinor() const { return m_MinorVersion; }          ///< The minor version of the engine
        [[nodiscard]] uint8_t GetEngineVersionPatch() const { return m_PatchVersion; }          ///< The patch version of the engine
        [[nodiscard]] uint32_t GetVkEngineVersion() const { return VK_MAKE_VERSION(m_MajorVersion, m_MinorVersion, m_PatchVersion); } ///< The Vulkan version of the engine

        [[nodiscard]] std::string GetAppName() const { return m_ApplicationName; }           ///< The name of the application
        [[nodiscard]] uint8_t GetAppVersionMajor() const { return m_ApplicationMajorVersion; }  ///< The major version of the application
        [[nodiscard]] uint8_t GetAppVersionMinor() const { return m_ApplicationMinorVersion; }  ///< The minor version of the application
        [[nodiscard]] uint8_t GetAppVersionPatch() const { return m_ApplicationPatchVersion; }  ///< The patch version of the application
        [[nodiscard]] uint32_t GetVkAppVersion() const { return VK_MAKE_VERSION(m_ApplicationMajorVersion, m_ApplicationMinorVersion, m_ApplicationPatchVersion); } ///< The Vulkan version of the application

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
         * @brief Sets the pipeline as loaded.
         *
         * When the pipeline is loaded, the settings should not be modified.
         */
        void PipelineLoaded() { m_PipelineLoaded = true; }

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
        void SetVSync(const bool vsync) { if (!m_PipelineLoaded) { m_VSync = vsync; } }                                                                 ///< Whether VSync is enabled

        void SetMaxLights(const uint16_t maxLights) { if (!m_PipelineLoaded) { m_MaxLights = maxLights; } }                                             ///< The maximum number of lights in the scene

        void SetMaxTextureSize(const uint16_t maxTextureSize) { if (!m_PipelineLoaded) { m_MaxTextureSize = maxTextureSize; } }                         ///< The maximum size of a texture
        void SetUseMipmaps(const bool useMipmaps) { if (!m_PipelineLoaded) { m_UseMipmaps = useMipmaps; } }                                             ///< Whether to use mipmaps for textures
        void SetAnisotropicFiltering(const bool anisotropicFiltering) { if (!m_PipelineLoaded) { m_AnisotropicFiltering = anisotropicFiltering; } }     ///< Whether to use anisotropic filtering for textures
        void SetMaxAnisotropy(const uint8_t maxAnisotropy) { if (!m_PipelineLoaded) { m_MaxAnisotropy = maxAnisotropy; } }                              ///< The maximum anisotropy level
        void SetUseBindlessTextures(const bool useBindlessTextures) { if (!m_PipelineLoaded) { m_UseBindlessTextures = useBindlessTextures; } }         ///< Whether to use bindless textures
        void SetMaxTextureSamplers(const uint16_t maxTextureSamplers) { if (!m_PipelineLoaded) { m_MaxTextureSamplers = maxTextureSamplers; } }         ///< The maximum number of texture samplers

        void ResetWindowFlags(const uint8_t windowID) { m_WindowSettings[windowID].ResetFlags(); }                                                      ///< Resets the flags for a window

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

        bool m_PipelineLoaded = false;             ///< Whether the pipeline has been loaded // TODO

        // Engine settings
        std::string m_EngineName = "Liara Engine";  ///< The name of the engine
        const uint8_t m_MajorVersion = 0;           ///< The major version of the application
        const uint8_t m_MinorVersion = 16;          ///< The minor version of the application
        const uint8_t m_PatchVersion = 0;           ///< The patch version of the application

        // Application settings
        std::string m_ApplicationName = "Liara Application";  ///< The name of the application
        const uint8_t m_ApplicationMajorVersion = 0;          ///< The major version of the application
        const uint8_t m_ApplicationMinorVersion = 1;          ///< The minor version of the application
        const uint8_t m_ApplicationPatchVersion = 0;          ///< The patch version of the application

        // Window settings
        std::unordered_map<uint8_t, WindowSettings> m_WindowSettings = {};  ///< The settings for each window
        bool m_VSync = true;                       ///< Whether VSync is enabled // TODO

        // Graphics settings
        uint16_t m_MaxLights = 10;                 ///< The maximum number of lights in the scene

        // Textures settings // TODO
        uint16_t m_MaxTextureSize = 2048;          ///< The maximum size of a texture
        bool m_UseMipmaps = true;                  ///< Whether to use mipmaps for textures
        bool m_AnisotropicFiltering = true;        ///< Whether to use anisotropic filtering for textures
        uint16_t m_MaxAnisotropy = 16;             ///< The maximum anisotropy level
        bool m_UseBindlessTextures = true;         ///< Whether to use bindless textures
        uint16_t m_MaxTextureSamplers = 16;        ///< The maximum number of texture samplers

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

        std::unordered_map<std::string, std::function<void(std::string)>> m_Setters{    ///< Map of setters for the settings
            {"vsync", [this](const std::string& value) { SetBool(m_VSync, value); }},
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