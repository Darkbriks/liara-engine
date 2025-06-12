#pragma once

#include <fstream>
#include <mutex>
#include <sstream>
#include <string_view>
#include <format>

#include "Liara_SettingSerializer.h"
#include "Liara_SettingsManager.h"

namespace Liara::Core {
    template<typename T>
    [[nodiscard]] bool Liara_SettingsManager::SettingStorage::HoldsType() const {
        if constexpr (FastSettingType<T>) {
            return std::holds_alternative<FastSettingEntry<T> >(data);
        } else {
            if (auto *flex = std::get_if<FlexibleSettingEntry>(&data)) {
                return flex->type_hash == std::hash<std::string>{}(typeid(T).name());
            }
            return false;
        }
    }

    template<typename T>
    void Liara_SettingsManager::RegisterSetting(const std::string_view name, T &&default_value, SettingFlags flags, const bool overwrite) {
        std::unique_lock lock(m_Mutex);
        if (!overwrite && m_Settings.contains(std::string(name))) { return; }

        if constexpr (FastSettingType<std::decay_t<T> >) {
            m_Settings[std::string(name)] = SettingStorage(FastSettingEntry<std::decay_t<T>>(std::forward<T>(default_value), flags));
        } else {
            m_Settings[std::string(name)] = SettingStorage(
                    FlexibleSettingEntry(
                    std::make_any<T>(std::forward<T>(default_value)),
                    std::hash<std::string>{}(typeid(T).name()),
                    flags
                )
            );
        }
    }

    template<typename T>
    [[nodiscard]] T Liara_SettingsManager::Get(const std::string_view name) const {
        std::shared_lock lock(m_Mutex);

        const auto it = m_Settings.find(std::string(name));
        if (it == m_Settings.end()) {
            throw std::runtime_error("Setting not found: " + std::string(name));
        }

        if constexpr (FastSettingType<T>) {
            if (auto *entry = std::get_if<FastSettingEntry<T> >(&it->second.data)) {
                return entry->value;
            }
        } else {
            if (auto *entry = std::get_if<FlexibleSettingEntry>(&it->second.data)) {
                if (entry->type_hash == std::hash<std::string>{}(typeid(T).name())) {
                    return std::any_cast<T>(entry->value);
                }
            }
        }

        throw std::runtime_error("Setting type mismatch for: " + std::string(name));
    }

    template<typename T>
    bool Liara_SettingsManager::Set(const std::string_view name, const T &value) {
        std::unique_lock lock(m_Mutex);

        const auto it = m_Settings.find(std::string(name));
        if (it == m_Settings.end()) return false;

        // Vérification des flags et du type + mise à jour
        bool success = false;
        std::vector<std::unique_ptr<ISettingObserver> > *observers = nullptr;

        if constexpr (FastSettingType<T>) {
            if (auto *entry = std::get_if<FastSettingEntry<T> >(&it->second.data)) {
                if (static_cast<uint32_t>(entry->flags) & static_cast<uint32_t>(SettingFlags::RuntimeModifiable)) {
                    entry->value = value;
                    observers = &entry->observers;
                    success = true;
                }
            }
        } else {
            if (auto *entry = std::get_if<FlexibleSettingEntry>(&it->second.data)) {
                if (static_cast<uint32_t>(entry->flags) & static_cast<uint32_t>(SettingFlags::RuntimeModifiable) &&
                    entry->type_hash == std::hash<std::string>{}(typeid(T).name())) {
                    entry->value = value;
                    observers = &entry->observers;
                    success = true;
                }
            }
        }

        if (success && observers) {
            // Copier les observers pour notifier sans lock
            std::vector<ISettingObserver *> observers_copy;
            observers_copy.reserve(observers->size());
            for (const auto &obs: *observers) {
                observers_copy.push_back(obs.get());
            }

            lock.unlock();

            // Notifier
            std::any any_value;
            if constexpr (FastSettingType<T>) { any_value = value; }
            else { any_value = std::make_any<T>(value); }
            for (auto *observer: observers_copy) { observer->Notify(any_value); }
        }

        return success;
    }

    template<typename T>
    void Liara_SettingsManager::Subscribe(const std::string_view name, std::function<void(const T &)> callback) {
        std::unique_lock lock(m_Mutex);

        const auto it = m_Settings.find(std::string(name));
        if (it == m_Settings.end()) { return; }

        auto observer = std::make_unique<TypedObserver<T> >(std::move(callback));

        std::visit([&observer](auto &entry) {
            entry.observers.emplace_back(std::move(observer));
        }, it->second.data);
    }

    template<typename T>
    bool Liara_SettingsManager::HasSetting(const std::string_view name) const {
        std::shared_lock lock(m_Mutex);

        const auto it = m_Settings.find(std::string(name));
        if (it == m_Settings.end()) return false;

        return it->second.HoldsType<T>();
    }

    template<typename Entry>
    bool Liara_SettingsManager::SerializeFastEntry(std::ofstream& file, const std::string& name, const Entry& entry) const {
        using ValueType = typename Entry::value_type;

        if constexpr (SerializableType<ValueType>) {
            auto serialized = SettingSerializer<ValueType>::serialize(entry.value);
            file << name << "=" << serialized << "\n";
            return true;
        } else {
            return false;
        }
    }

    template<typename Entry>
    bool Liara_SettingsManager::DeserializeFastEntry(Entry& entry, const std::string& value) {
        using ValueType = typename Entry::value_type;

        if constexpr (SerializableType<ValueType>) {
            ValueType new_value;
            if (SettingSerializer<ValueType>::deserialize(value, new_value)) {
                entry.value = new_value;
                return true;
            }
        }
        return false;
    }
}
