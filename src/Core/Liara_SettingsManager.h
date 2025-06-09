#pragma once

#include <any>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <variant>
#include <type_traits>

#include "fmt/printf.h"

namespace Liara::Core {
    enum class SettingFlags : uint32_t {
        None = 0,
        RuntimeModifiable = 1 << 0,
        Serializable = 1 << 1,
        Default = RuntimeModifiable | Serializable
    };

    inline SettingFlags operator|(SettingFlags lhs, SettingFlags rhs) {
        return static_cast<SettingFlags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    }

    template<typename T>
    concept FastSettingType = (std::is_arithmetic_v<T> || std::is_enum_v<T> || std::is_same_v<T, std::string>) && std::is_trivially_copyable_v<T>;

    // Interface commune pour les observers
    class ISettingObserver {
    public:
        virtual ~ISettingObserver() = default;
        virtual void Notify(const std::any& value) = 0;
    };

    template<typename T>
    class TypedObserver final : public ISettingObserver {
    public:
        explicit TypedObserver(std::function<void(const T&)> callback) : m_callback(std::move(callback)) {}

        void Notify(const std::any& value) override {
            try {
                if constexpr (FastSettingType<T>) {
                    m_callback(std::any_cast<T>(value));
                } else {
                    m_callback(std::any_cast<const T&>(value));
                }
            } catch (const std::bad_any_cast&) {
                // Type mismatch, ignore silently
                fmt::print("Type mismatch in observer notification for type {}\n", typeid(T).name());
            }
        }

    private:
        std::function<void(const T&)> m_callback;
    };

    class SettingsManager {
    private:
        // Storage rapide pour les types fréquents
        template<FastSettingType T>
        struct FastSettingEntry {
            using value_type = T;
            T value;
            SettingFlags flags;
            std::vector<std::unique_ptr<ISettingObserver>> observers;

            explicit FastSettingEntry(T default_value, const SettingFlags setting_flags = SettingFlags::Default)
                : value(default_value), flags(setting_flags) {}
        };

        // Storage flexible pour les types arbitraires
        struct FlexibleSettingEntry {
            std::any value;
            SettingFlags flags;
            size_t type_hash;
            std::vector<std::unique_ptr<ISettingObserver>> observers;

            explicit FlexibleSettingEntry(std::any default_value, const SettingFlags setting_flags = SettingFlags::Default)
                : value(std::move(default_value)), flags(setting_flags), type_hash(value.type().hash_code()) {}
            explicit FlexibleSettingEntry(std::any default_value, const size_t type_hash, const SettingFlags setting_flags = SettingFlags::Default)
                : value(std::move(default_value)), flags(setting_flags), type_hash(type_hash) {}
        };

        // Container unifié
        struct SettingStorage {
            std::variant<
                FastSettingEntry<bool>,
                FastSettingEntry<int>,
                FastSettingEntry<uint32_t>,
                FastSettingEntry<float>,
                FastSettingEntry<double>,
                // FastSettingEntry<std::string>,
                FlexibleSettingEntry
            > data;

            SettingStorage(): data(FlexibleSettingEntry(std::string(), SettingFlags::Default)) {}
            explicit SettingStorage(FastSettingEntry<bool> entry) : data(std::move(entry)) {}
            explicit SettingStorage(FastSettingEntry<int> entry) : data(std::move(entry)) {}
            explicit SettingStorage(FastSettingEntry<uint32_t> entry) : data(std::move(entry)) {}
            explicit SettingStorage(FastSettingEntry<float> entry) : data(std::move(entry)) {}
            explicit SettingStorage(FastSettingEntry<double> entry) : data(std::move(entry)) {}
            explicit SettingStorage(FlexibleSettingEntry entry) : data(std::move(entry)) {}
            SettingStorage(std::any default_value, const size_t type_hash, const SettingFlags flags = SettingFlags::Default)
                : data(FlexibleSettingEntry(std::move(default_value), type_hash, flags)) {}

            template<typename T>
            [[nodiscard]] bool HoldsType() const;
        };

    public:
        SettingsManager();

        template<typename T>
        void RegisterSetting(std::string_view name, T&& default_value, SettingFlags flags = SettingFlags::Default, bool overwrite = false);

        template<typename T>
        [[nodiscard]] T Get(std::string_view name) const;

        template<typename T>
        bool Set(std::string_view name, const T& value);

        template<typename T>
        void Subscribe(std::string_view name, std::function<void(const T&)> callback);

        // Helpers pour les types courants (interface simplifiée)
        bool GetBool(const std::string_view name) const { return Get<bool>(name); }
        int GetInt(const std::string_view name) const { return Get<int>(name); }
        uint32_t GetUInt(const std::string_view name) const { return Get<uint32_t>(name); }
        float GetFloat(const std::string_view name) const { return Get<float>(name); }
        std::string GetString(const std::string_view name) const { return Get<std::string>(name); }

        void SetBool(const std::string_view name, const bool value) { Set(name, value); }
        void SetInt(const std::string_view name, const int value) { Set(name, value); }
        void SetUInt(const std::string_view name, const uint32_t value) { Set(name, value); }
        void SetFloat(const std::string_view name, const float value) { Set(name, value); }
        void SetString(const std::string_view name, const std::string& value) { Set(name, value); }

        // Utility pour debug/introspection
        std::vector<std::string> GetAllSettingNames() const;

        template<typename T>
        bool HasSetting(std::string_view name) const;

        bool save_to_file(const std::string& filename, bool overwrite = true) const;
        bool load_from_file(const std::string& filename);

    private:

        template<typename Entry>
        bool serialize_fast_entry(std::ofstream& file, const std::string& name, const Entry& entry) const;
        static bool serialize_flexible_entry(std::ofstream& file, const std::string& name, const FlexibleSettingEntry& entry) ;

        bool deserialize_setting(const std::string& key, const std::string& value);
        template<typename Entry>
        bool deserialize_fast_entry(Entry& entry, const std::string& value);
        static bool deserialize_flexible_entry(const FlexibleSettingEntry& entry, const std::string& value);
        static bool is_serializable(const SettingStorage& storage);

        static std::string trim(const std::string& str);

        mutable std::shared_mutex mutex_;
        std::unordered_map<std::string, SettingStorage> settings_;
    };
}

#include "Liara_SettingsManager.tpp"