#include "Liara_SettingsManager.h"

#include <fstream>
#include <ranges>

#include <vulkan/vulkan_core.h>

#include "Config.h"
#include "Liara_SettingSerializer.h"
#include "Plateform/Liara_Window.h"

namespace Liara::Core {
    Liara_SettingsManager::Liara_SettingsManager() {
        // Default settings
        RegisterSetting("global.engine_name", ENGINE_NAME, SettingFlags::None);
        RegisterSetting("global.engine_version", VK_MAKE_VERSION(ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH), SettingFlags::None);

        RegisterSetting("global.app_name", APP_NAME, SettingFlags::None);
        RegisterSetting("global.app_version", VK_MAKE_VERSION(APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH), SettingFlags::None);


        /**
         * The preferred present mode.
         * Most common values:
         *      - VK_PRESENT_MODE_IMMEDIATE_KHR: The image is transferred to the screen immediately, which may result in tearing, but lower latency.
         *      - VK_PRESENT_MODE_FIFO_KHR: Uses a FIFO queue to present images. The display is synchronized with the vertical blanking period. No tearing, but higher latency.
         *      - VK_PRESENT_MODE_MAILBOX_KHR: Uses a mailbox queue to present images. The display is synchronized with the vertical blanking period. No tearing, lower latency than FIFO, but can need more resources.
         */
        RegisterSetting("graphics.present_mode", static_cast<uint32_t>(VK_PRESENT_MODE_MAILBOX_KHR), SettingFlags::Serializable);
        RegisterSetting("graphics.vsync", true, SettingFlags::Default);

        RegisterSetting("texture.use_anisotropic_filtering", true, SettingFlags::Serializable);
        RegisterSetting("texture.max_anisotropy", 16u, SettingFlags::Default);
        RegisterSetting("texture.use_bindless_textures", false, SettingFlags::Serializable);
        RegisterSetting("texture.max_size", 4096u, SettingFlags::Serializable);
        RegisterSetting("texture.use_mipmaps", true, SettingFlags::Serializable);
    }

    std::vector<std::string> Liara_SettingsManager::GetAllSettingNames() const {
        std::shared_lock lock(m_Mutex);
        std::vector<std::string> names;
        names.reserve(m_Settings.size());

        for (const auto &name: m_Settings | std::views::keys) {
            names.push_back(name);
        }

        return names;
    }

    bool Liara_SettingsManager::SaveToFile(const std::string& filename, const bool overwrite) const {
        if (!overwrite && std::ifstream(filename).good()) {
            return false;
        }

        std::ofstream file(filename);
        if (!file) return false;

        file << "# Liara Engine Settings\n";
        file << "# Auto-generated file\n\n";

        std::shared_lock lock(m_Mutex);

        for (const auto& [name, storage] : m_Settings) {
            if (!IsSerializable(storage)) { continue; }

            const bool success = std::visit([&]<typename T0>(const T0& entry) -> bool {
                using EntryType = std::decay_t<T0>;

                if constexpr (std::is_same_v<EntryType, FlexibleSettingEntry>) {
                    return SerializeFlexibleEntry(file, name, entry);
                } else {
                    return SerializeFastEntry(file, name, entry);
                }
            }, storage.data);

            if (!success) { file << "# Failed to serialize: " << name << "\n"; }
        }

        return file.good();
    }

    bool Liara_SettingsManager::LoadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) return false;

        std::string line;
        size_t line_number = 0;

        while (std::getline(file, line)) {
            ++line_number;

            // Ignorer commentaires et lignes vides
            line = Trim(line);
            if (line.empty() || line[0] == '#') { continue; }

            // Parser "key=value"
            size_t eq_pos = line.find('=');
            if (eq_pos == std::string::npos) { continue; }

            std::string key = Trim(line.substr(0, eq_pos));

            if (std::string value = Trim(line.substr(eq_pos + 1)); !DeserializeSetting(key, value)) {
                std::fprintf(stderr, "Warning: Failed to deserialize setting '%s' at line %zu\n",
                           key.c_str(), line_number);
            }
        }

        return true;
    }

    bool Liara_SettingsManager::SerializeFlexibleEntry(std::ofstream& file, const std::string& name, const FlexibleSettingEntry& entry) {
        // TODO: Check why cast to ISettingSerializable fails
        // Si l'entrée est sérialisable, utiliser la méthode de sérialisation
        if (const auto serializable = std::any_cast<ISettingSerializable*>(&entry.value)) {
            file << name << "=" << (*serializable)->serialize() << "\n";
            return true;
        }

        return false;
    }

    bool Liara_SettingsManager::DeserializeSetting(const std::string& key, const std::string& value) {
        const auto it = m_Settings.find(key);
        if (it == m_Settings.end()) {
            // Setting inconnu, créer un setting string par défaut
            RegisterSetting(key, value, SettingFlags::Default, true);
            return true;
        }

        return std::visit([&]<typename T0>(T0& entry) -> bool {
            using EntryType = std::decay_t<T0>;

            if constexpr (std::is_same_v<EntryType, FlexibleSettingEntry>) {
                return DeserializeFlexibleEntry(entry, value);
            } else {
                return DeserializeFastEntry(entry, value);
            }
        }, it->second.data);
    }

    bool Liara_SettingsManager::DeserializeFlexibleEntry(const FlexibleSettingEntry& entry, const std::string& value) {
        // TODO: Check why cast to ISettingSerializable fails
        if (const auto* serializable = std::any_cast<ISettingSerializable*>(&entry.value)) {
            return (*serializable)->deserialize(value);
        }
        return false;
    }

    bool Liara_SettingsManager::IsSerializable(const SettingStorage& storage) {
        return std::visit([](const auto& entry) -> bool {
            return static_cast<uint32_t>(entry.flags) & static_cast<uint32_t>(SettingFlags::Serializable);
        }, storage.data);
    }

    std::string Liara_SettingsManager::Trim(const std::string& str) {
        const auto start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";

        const auto end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }
}
