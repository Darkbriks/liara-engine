#include "Liara_Settings.h"

#include <fstream>
#include <fmt/core.h>
#include <filesystem>

#include "Plateform/Liara_Window.h"

namespace Liara
{
    void Liara_Settings::SetWindowWidth(const uint8_t windowID, const uint16_t width)
    {
        if (width > 0 && m_WindowSettings[windowID].width != width)
        {
            m_WindowSettings[windowID].width = width;
            m_WindowSettings[windowID].resized = true;
            m_WindowSettings[windowID].window_state_change = true;
        }
    }

    void Liara_Settings::SetWindowHeight(const uint8_t windowID, const uint16_t height)
    {
        if (height > 0 && m_WindowSettings[windowID].height != height)
        {
            m_WindowSettings[windowID].height = height;
            m_WindowSettings[windowID].resized = true;
            m_WindowSettings[windowID].window_state_change = true;
        }
    }

    void Liara_Settings::SetWindowFullscreen(const uint8_t windowID, const bool fullscreen)
    {
        if (m_WindowSettings[windowID].fullscreen != fullscreen)
        {
            m_WindowSettings[windowID].fullscreen = fullscreen;
            m_WindowSettings[windowID].fullscreen_change = true;
            m_WindowSettings[windowID].window_state_change = true;
        }
    }

    bool Liara_Settings::LoadFromFile(const std::string& filename)
    {
        if (std::ifstream file(filename); file.is_open())
        {
            // Read the file line by line
            std::string line;
            while (std::getline(file, line))
            {
                if (line.empty()) { continue; }
                if (line[0] == '#') { continue; }

                if (line[0] == '[')
                {
                    const size_t closingBracket = line.find(']');
                    if (closingBracket == std::string::npos) { continue; }

                    const size_t underscore = line.find('_');
                    if (underscore == std::string::npos) { continue; }

                    const std::string section = line.substr(1, underscore - 1);
                    const std::string value = line.substr(underscore + 1, closingBracket - underscore - 1);

                    if (section == "window")
                    {
                        const auto windowID = static_cast<uint8_t>(std::stoi(value));
                        LoadWindowSettings(file, windowID);
                    }

                    continue;
                }

                // Find the equal sign
                const size_t equalSign = line.find('=');
                if (equalSign == std::string::npos) { continue; }

                // Extract the key and value
                const std::string key = line.substr(0, equalSign);
                const std::string value = line.substr(equalSign + 1);

                // Parse the value
                if (const auto it = m_Setters.find(key); it != m_Setters.end())
                {
                    it->second(value);
                }
            }

            return true;
        }

        return false;
    }

    void Liara_Settings::LoadWindowSettings(std::ifstream& file, const uint8_t windowID)
    {
        std::string line;
        WindowSettings settings;

        while (std::getline(file, line))
        {
            if (line.empty()) { continue; }
            if (line[0] == '#') { continue; }

            if (line[0] == '[') { break; }

            const size_t equalSign = line.find('=');
            if (equalSign == std::string::npos) { continue; }

            const std::string key = line.substr(0, equalSign);
            const std::string value = line.substr(equalSign + 1);

            if (key == "width") { SetUint16(settings.width, value); }
            else if (key == "height") { SetUint16(settings.height, value); }
            else if (key == "x_pos") { SetInt(settings.x_pos, value); }
            else if (key == "y_pos") { SetInt(settings.y_pos, value); }
            else if (key == "fullscreen") { SetBool(settings.fullscreen, value); }
            else if (key == "resizable") { SetBool(settings.resizable, value); }
        }

        m_WindowSettings[windowID] = settings;
    }


    bool Liara_Settings::SaveToFile(const std::string& filename, const bool overwrite, const bool create) const
    {
        if (std::ifstream file(filename); file.is_open())
        {
            if (!overwrite)
            {
                fmt::print(stderr, "Liara_Settings::SaveToFile: File already exists: {}\n", filename);
                return false;
            }
            file.close();
            return WriteToFile(filename);
        }
        else
        {
            if (!create)
            {
                fmt::print(stderr, "Liara_Settings::SaveToFile: File does not exist: {}\n", filename);
                return false;
            }
            file.close();
            return WriteToFile(filename);
        }
    }

    bool Liara_Settings::WriteToFile(const std::string& filename) const
    {
        if (std::ofstream newFile(filename); newFile.is_open())
        {
            newFile << "# Liara Engine Settings\n";
            newFile << "# This file was automatically generated by the engine.\n";
            newFile << "# Be careful when editing this file manually, and consider making a backup.\n";
            newFile << "\n";

            for (const auto& [key, value] : m_WindowSettings)
            {
                newFile << "[window_" << static_cast<int>(key) << "]\n";
                newFile << "width=" << value.width << "\n";
                newFile << "height=" << value.height << "\n";
                newFile << "x_pos=" << value.x_pos << "\n";
                newFile << "y_pos=" << value.y_pos << "\n";
                newFile << "fullscreen=" << (value.fullscreen ? "true" : "false") << "\n";
                newFile << "resizable=" << (value.resizable ? "true" : "false") << "\n";
                newFile << "[/window_" << static_cast<int>(key) << "]\n";
            }

            newFile << "vsync=" << (m_VSync ? "true" : "false") << "\n";
            newFile << "max_lights=" << m_MaxLights << "\n";
            newFile << "max_texture_size=" << m_MaxTextureSize << "\n";
            newFile << "use_mipmaps=" << (m_UseMipmaps ? "true" : "false") << "\n";
            newFile << "use_anisotropic_filtering=" << (m_AnisotropicFiltering ? "true" : "false") << "\n";
            newFile << "max_anisotropy=" << m_MaxAnisotropy << "\n";
            newFile << "use_bindless_textures=" << (m_UseBindlessTextures ? "true" : "false") << "\n";
            newFile << "max_texture_samplers=" << m_MaxTextureSamplers << "\n";

            fmt::print("Settings saved to: {}\n", std::filesystem::absolute(filename).string());

            return true;
        }
        return false;
    }

    void Liara_Settings::SetString(std::string& key, const std::string& value) { key = value; }
    void Liara_Settings::SetBool(bool& key, const std::string& value) { key = value == "true"; }
    void Liara_Settings::SetInt(int& key, const std::string& value) { key = std::stoi(value); }
    void Liara_Settings::SetUint8(uint8_t& key, const std::string& value) { key = static_cast<uint8_t>(std::stoi(value)); }
    void Liara_Settings::SetUint16(uint16_t& key, const std::string& value) { key = static_cast<uint16_t>(std::stoi(value)); }
    void Liara_Settings::SetUint32(uint32_t& key, const std::string& value) { key = static_cast<uint32_t>(std::stoi(value)); }

}