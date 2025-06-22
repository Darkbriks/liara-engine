#include "Liara_SettingsManager.h"

#include <vulkan/vulkan_core.h>

#include <any>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <ranges>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "ApplicationInfo.h"
#include "Config.h"
#include "Liara_SettingSerializer.h"

namespace Liara::Core
{
    Liara_SettingsManager::Liara_SettingsManager(const ApplicationInfo& appInfo) {
        /**
         * The preferred present mode.
         * Most common values:
         *      - VK_PRESENT_MODE_IMMEDIATE_KHR: The image is transferred to the screen immediately, which may result in
         * tearing, but lower latency.
         *      - VK_PRESENT_MODE_FIFO_KHR: Uses a FIFO queue to present images. The display is synchronized with the
         * vertical blanking period. No tearing, but higher latency.
         *      - VK_PRESENT_MODE_MAILBOX_KHR: Uses a mailbox queue to present images. The display is synchronized with
         * the vertical blanking period. No tearing, lower latency than FIFO, but can need more resources.
         */
        RegisterSetting(
            "graphics.present_mode", static_cast<uint32_t>(VK_PRESENT_MODE_MAILBOX_KHR), SettingFlags::SERIALIZABLE);
        RegisterSetting("graphics.vsync", true, SettingFlags::DEFAULT);

        RegisterSetting("texture.use_anisotropic_filtering", true, SettingFlags::SERIALIZABLE);
        RegisterSetting("texture.max_anisotropy", 16u, SettingFlags::DEFAULT);
        RegisterSetting("texture.use_bindless_textures", false, SettingFlags::SERIALIZABLE);
        RegisterSetting("texture.max_size", 4096u, SettingFlags::SERIALIZABLE);
        RegisterSetting("texture.use_mipmaps", true, SettingFlags::SERIALIZABLE);

        // Register application information settings
        static_assert(IsValidAppInfo({}), "DEFAULT ApplicationInfo must be valid");
        if (!IsValidAppInfo(appInfo)) { throw std::invalid_argument("Invalid ApplicationInfo provided"); }

        RegisterSetting("global.app_name", std::string(appInfo.name), SettingFlags::NONE);
        RegisterSetting("global.app_display_name", std::string(appInfo.GetDisplayName()), SettingFlags::NONE);
        RegisterSetting("global.app_description", std::string(appInfo.description), SettingFlags::NONE);
        RegisterSetting("global.app_version", appInfo.version.Packed(), SettingFlags::NONE);
        RegisterSetting("global.app_version_string", appInfo.version.ToString(), SettingFlags::NONE);

        if (!appInfo.organization.empty()) {
            RegisterSetting("global.app_organization", std::string(appInfo.organization), SettingFlags::NONE);
        }
        if (!appInfo.website.empty()) {
            RegisterSetting("global.app_website", std::string(appInfo.website), SettingFlags::NONE);
        }
        if (!appInfo.copyright.empty()) {
            RegisterSetting("global.app_copyright", std::string(appInfo.copyright), SettingFlags::NONE);
        }

        RegisterSetting("global.app_build_config", std::string(appInfo.buildConfig), SettingFlags::NONE);
        RegisterSetting("global.app_target_platform", std::string(appInfo.targetPlatform), SettingFlags::NONE);

        RegisterSetting("global.engine_name", std::string(ENGINE_NAME), SettingFlags::NONE);
        RegisterSetting("global.engine_version",
                        VK_MAKE_VERSION(ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH),
                        SettingFlags::NONE);
    }

    std::vector<std::string> Liara_SettingsManager::GetAllSettingNames() const {
        const std::shared_lock lock(m_Mutex);
        std::vector<std::string> names;
        names.reserve(m_Settings.size());

        for (const auto& name : m_Settings | std::views::keys) { names.push_back(name); }

        return names;
    }

    bool Liara_SettingsManager::SaveToFile(const std::string& filename, const bool overwrite) const {
        if (!overwrite && std::ifstream(filename).good()) { return false; }

        std::ofstream file(filename);
        if (!file) return false;

        file << "# Liara Engine Settings\n";
        file << "# Auto-generated file\n\n";

        const std::shared_lock lock(m_Mutex);

        for (const auto& [name, storage] : m_Settings) {
            if (!IsSerializable(storage)) { continue; }

            const bool success = std::visit(
                [&]<typename T0>(const T0& entry) -> bool {
                    using EntryType = std::decay_t<T0>;

                    if constexpr (std::is_same_v<EntryType, Liara_FlexibleSettingEntry>) {
                        return SerializeFlexibleEntry(file, name, entry);
                    }
                    else { return SerializeFastEntry(file, name, entry); }
                },
                storage.data);

            if (!success) { file << "# Failed to serialize: " << name << "\n"; }
        }

        return file.good();
    }

    bool Liara_SettingsManager::LoadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) return false;

        std::string line;
        size_t lineNumber = 0;

        while (std::getline(file, line)) {
            ++lineNumber;

            // Ignorer commentaires et lignes vides
            line = Trim(line);
            if (line.empty() || line[0] == '#') { continue; }

            // Parser "key=value"
            const size_t eqPos = line.find('=');
            if (eqPos == std::string::npos) { continue; }

            const std::string key = Trim(line.substr(0, eqPos));

            if (const std::string value = Trim(line.substr(eqPos + 1)); !DeserializeSetting(key, value)) {
                std::fprintf(
                    stderr, "Warning: Failed to deserialize setting '%s' at line %zu\n", key.c_str(), lineNumber);
            }
        }

        return true;
    }

    bool Liara_SettingsManager::SerializeFlexibleEntry(std::ofstream& file,
                                                       const std::string& name,
                                                       const Liara_FlexibleSettingEntry& entry) {
        // TODO: Check why cast to ISettingSerializable fails
        // Si l'entrée est sérialisable, utiliser la méthode de sérialisation
        if (const auto* const serializable = std::any_cast<ISettingSerializable*>(&entry.value)) {
            file << name << "=" << (*serializable)->serialize() << "\n";
            return true;
        }

        return false;
    }

    bool Liara_SettingsManager::DeserializeSetting(const std::string& key, const std::string& value) {
        const auto it = m_Settings.find(key);
        if (it == m_Settings.end()) {
            // Setting inconnu, créer un setting string par défaut
            RegisterSetting(key, value, SettingFlags::DEFAULT, true);
            return true;
        }

        return std::visit(
            [&]<typename T0>(T0& entry) -> bool {
                using EntryType = std::decay_t<T0>;

                if constexpr (std::is_same_v<EntryType, Liara_FlexibleSettingEntry>) {
                    return DeserializeFlexibleEntry(entry, value);
                }
                else { return DeserializeFastEntry(entry, value); }
            },
            it->second.data);
    }

    bool Liara_SettingsManager::DeserializeFlexibleEntry(const Liara_FlexibleSettingEntry& entry,
                                                         const std::string& value) {
        // TODO: Check why cast to ISettingSerializable fails
        if (const auto* serializable = std::any_cast<ISettingSerializable*>(&entry.value)) {
            return (*serializable)->deserialize(value);
        }
        return false;
    }

    bool Liara_SettingsManager::IsSerializable(const Liara_SettingStorage& storage) {
        return std::visit(
            [](const auto& entry) -> bool {
                return static_cast<uint32_t>(entry.flags) & static_cast<uint32_t>(SettingFlags::SERIALIZABLE);
            },
            storage.data);
    }

    std::string Liara_SettingsManager::Trim(const std::string& str) {
        const auto start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";

        const auto end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }
}
