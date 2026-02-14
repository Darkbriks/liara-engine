#pragma once

#include "Liara_SettingSerializer.h"

#include <any>
#include <cstdint>
#include <format>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace Liara::Core {
    template <typename T> [[nodiscard]] bool Liara_SettingsManager::Liara_SettingStorage::HoldsType() const {
        if constexpr (FastSettingType<T>) { return std::holds_alternative<Liara_FastSettingEntry<T>>(data); }
        else {
            if (const auto* flex = std::get_if<Liara_FlexibleSettingEntry>(&data)) {
                return flex->typeHash == std::hash<std::string>{}(typeid(T).name());
            }
            return false;
        }
    }

    template <typename T>
    void Liara_SettingsManager::RegisterSetting(const std::string_view name,
                                                T&& defaultValue,
                                                SettingFlags flags,
                                                const bool overwrite) {
        std::unique_lock const lock(m_Mutex);
        if (!overwrite && m_Settings.contains(std::string(name))) { return; }

        if constexpr (FastSettingType<std::decay_t<T>>) {
            m_Settings[std::string(name)] =
                Liara_SettingStorage(Liara_FastSettingEntry<std::decay_t<T>>(std::forward<T>(defaultValue), flags));
        } else {
            if constexpr (std::is_base_of_v<ISettingSerializable, std::decay_t<T>>) {
                // If the type is serializable, we don't use std::any to store the value, but a pointer to the serializable object
                // This avoids copies and sync issues.
                auto serializablePtr = std::make_shared<std::decay_t<T>>(std::forward<T>(defaultValue));

                // Lambda to create std::any from the serializablePtr when needed
                auto anyCreator = [serializablePtr]() -> std::any {
                    return std::make_any<std::decay_t<T>>(*serializablePtr);
                };

                m_Settings[std::string(name)] = Liara_SettingStorage(
                    Liara_FlexibleSettingEntry(
                        std::any{}, // Empty, value is in serializablePtr
                        std::hash<std::string>{}(typeid(T).name()),
                        flags,
                        serializablePtr,
                        anyCreator));
            } else {
                m_Settings[std::string(name)] = Liara_SettingStorage(
                    Liara_FlexibleSettingEntry(
                        std::make_any<T>(std::forward<T>(defaultValue)),
                        std::hash<std::string>{}(typeid(T).name()),
                        flags));
            }
        }
    }

    template <typename T>
    [[nodiscard]] T Liara_SettingsManager::Get(const std::string_view name) const {
        std::shared_lock const lock(m_Mutex);

        const auto it = m_Settings.find(std::string(name));
        LIARA_CHECK_RUNTIME(it != m_Settings.end(), LogCore, "Setting not found: {}", std::string(name));

        if constexpr (FastSettingType<T>) {
            if (auto* entry = std::get_if<Liara_FastSettingEntry<T>>(&it->second.data)) { return entry->value; }
        } else {
            if (const auto* entry = std::get_if<Liara_FlexibleSettingEntry>(&it->second.data)) {
                if (entry->typeHash == std::hash<std::string>{}(typeid(T).name())) {

                    // If the setting is serializable, we need to get the value from the serializablePtr
                    if (entry->serializablePtr) {
                        if (auto* typed = dynamic_cast<T*>(entry->serializablePtr.get())) {
                            return *typed;
                        }
                        LIARA_THROW_RUNTIME_ERROR(LogCore,  "Failed to cast serializable setting '{}' to requested type", std::string(name));
                    }

                    // Else, return the value stored in std::any
                    return std::any_cast<T>(entry->value);
                }
            }
        }
        LIARA_THROW_RUNTIME_ERROR(LogCore, "Setting type mismatch for: {}", std::string(name));
    }

    template <typename T>
    bool Liara_SettingsManager::Set(const std::string_view name, const T& value) {
        std::unique_lock lock(m_Mutex);

        const auto it = m_Settings.find(std::string(name));
        if (it == m_Settings.end()) return false;

        // Vérification des flags et du type + mise à jour
        bool success = false;
        std::vector<std::unique_ptr<Liara_ISettingObserver>> const* observers = nullptr;

        if constexpr (FastSettingType<T>) {
            if (auto* entry = std::get_if<Liara_FastSettingEntry<T>>(&it->second.data)) {
                if (static_cast<uint32_t>(entry->flags) & static_cast<uint32_t>(SettingFlags::RUNTIME_MODIFIABLE)) {
                    entry->value = value;
                    observers = &entry->observers;
                    success = true;
                }
            }
        } else {
            if (auto* entry = std::get_if<Liara_FlexibleSettingEntry>(&it->second.data)) {
                if (static_cast<uint32_t>(entry->flags) & static_cast<uint32_t>(SettingFlags::RUNTIME_MODIFIABLE)
                    && entry->typeHash == std::hash<std::string>{}(typeid(T).name())) {

                    if (entry->serializablePtr) {
                        // If the setting is serializable, we need to set the value in the serializablePtr
                        if (auto* typed = dynamic_cast<T*>(entry->serializablePtr.get())) {
                            *typed = value;
                            observers = &entry->observers;
                            success = true;
                        }
                    } else {
                        // Else, we proceed to a normal update
                        entry->value = value;
                        observers = &entry->observers;
                        success = true;
                    }
                }
            }
        }

        if (success && observers) {
            // Copier les observers pour notifier sans lock
            std::vector<Liara_ISettingObserver*> observersCopy;
            observersCopy.reserve(observers->size());
            for (const auto &obs: *observers) {
                observersCopy.push_back(obs.get());
            }

            lock.unlock();

            // Notifier
            std::any anyValue;
            if constexpr (FastSettingType<T>) { anyValue = value; }
            else { anyValue = std::make_any<T>(value); }
            for (auto* observer : observersCopy) { observer->Notify(anyValue); }
        }

        return success;
    }

    template <typename T>
    void Liara_SettingsManager::Subscribe(const std::string_view name, std::function<void(const T&)> callback) {
        std::unique_lock const lock(m_Mutex);

        const auto it = m_Settings.find(std::string(name));
        if (it == m_Settings.end()) { return; }

        auto observer = std::make_unique<Liara_TypedObserver<T>>(std::move(callback));

        std::visit([&observer](auto& entry) { entry.observers.emplace_back(std::move(observer)); }, it->second.data);
    }

    template <typename T> bool Liara_SettingsManager::HasSetting(const std::string_view name) const {
        std::shared_lock const lock(m_Mutex);

        const auto it = m_Settings.find(std::string(name));
        if (it == m_Settings.end()) return false;

        return it->second.HoldsType<T>();
    }

    template <typename Entry>
    bool
    Liara_SettingsManager::SerializeFastEntry(std::ofstream& file, const std::string& name, const Entry& entry) const {
        using ValueType = typename Entry::value_type;

        if constexpr (SerializableType<ValueType>) {
            auto serialized = SettingSerializer<ValueType>::serialize(entry.value);
            file << name << "=" << serialized << "\n";
            return true;
        } else {
            return false;
        }
    }

    template <typename Entry> bool Liara_SettingsManager::DeserializeFastEntry(Entry& entry, const std::string& value) {
        using ValueType = typename Entry::value_type;

        if constexpr (SerializableType<ValueType>) {
            ValueType newValue;
            if (SettingSerializer<ValueType>::deserialize(value, newValue)) {
                entry.value = newValue;

                const std::any anyValue = newValue;
                for (const auto& observer : entry.observers) { observer->Notify(anyValue); }

                return true;
            }
        }
        return false;
    }
}
