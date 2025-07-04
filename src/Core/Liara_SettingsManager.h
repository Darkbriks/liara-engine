#pragma once

#include "fmt/core.h"

#include <any>
#include <cstdint>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace Liara::Core
{
    struct ApplicationInfo;

    enum class SettingFlags : uint8_t
    {
        NONE = 0,
        RUNTIME_MODIFIABLE = 1 << 0,
        SERIALIZABLE = 1 << 1,
        DEFAULT = RUNTIME_MODIFIABLE | SERIALIZABLE
    };

    inline SettingFlags operator|(SettingFlags lhs, SettingFlags rhs) {
        return static_cast<SettingFlags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    }

    template <typename T>
    concept FastSettingType = (std::is_arithmetic_v<T> || std::is_enum_v<T> || std::is_same_v<T, std::string>)
                              && std::is_trivially_copyable_v<T>;

    // Interface commune pour les observers
    class Liara_ISettingObserver
    {
    public:
        virtual ~Liara_ISettingObserver() = default;
        virtual void Notify(const std::any& value) = 0;
    };

    template <typename T> class Liara_TypedObserver final : public Liara_ISettingObserver
    {
    public:
        explicit Liara_TypedObserver(std::function<void(const T&)> callback)
            : m_callback(std::move(callback)) {}

        void Notify(const std::any& value) override {
            try {
                if constexpr (FastSettingType<T>) { m_callback(std::any_cast<T>(value)); }
                else { m_callback(std::any_cast<const T&>(value)); }
            }
            catch (const std::bad_any_cast&) {
                // Type mismatch, ignore silently
                fmt::print("Type mismatch in observer notification for type {}\n", typeid(T).name());
            }
        }

    private:
        std::function<void(const T&)> m_callback;
    };

    class Liara_SettingsManager
    {
    private:
        // Storage rapide pour les types fréquents
        template <FastSettingType T> struct Liara_FastSettingEntry
        {
            using value_type = T;
            T value;
            SettingFlags flags;
            std::vector<std::unique_ptr<Liara_ISettingObserver>> observers;

            explicit Liara_FastSettingEntry(T defaultValue, const SettingFlags settingFlags = SettingFlags::DEFAULT)
                : value(defaultValue)
                , flags(settingFlags) {}
        };

        // Storage flexible pour les types arbitraires
        struct Liara_FlexibleSettingEntry
        {
            std::any value;
            SettingFlags flags;
            size_t typeHash;
            std::vector<std::unique_ptr<Liara_ISettingObserver>> observers;

            explicit Liara_FlexibleSettingEntry(std::any defaultValue,
                                                const SettingFlags settingFlags = SettingFlags::DEFAULT)
                : value(std::move(defaultValue))
                , flags(settingFlags)
                , typeHash(value.type().hash_code()) {}
            explicit Liara_FlexibleSettingEntry(std::any defaultValue,
                                                const size_t typeHash,
                                                const SettingFlags settingFlags = SettingFlags::DEFAULT)
                : value(std::move(defaultValue))
                , flags(settingFlags)
                , typeHash(typeHash) {}
        };

        // Container unifié
        struct Liara_SettingStorage
        {
            std::variant<Liara_FastSettingEntry<bool>,
                         Liara_FastSettingEntry<int>,
                         Liara_FastSettingEntry<uint32_t>,
                         Liara_FastSettingEntry<float>,
                         Liara_FastSettingEntry<double>,
                         // FastSettingEntry<std::string>,
                         Liara_FlexibleSettingEntry>
                data;

            Liara_SettingStorage()
                : data(Liara_FlexibleSettingEntry(std::string(), SettingFlags::DEFAULT)) {}

            explicit Liara_SettingStorage(Liara_FastSettingEntry<bool> entry)
                : data(std::move(entry)) {}

            explicit Liara_SettingStorage(Liara_FastSettingEntry<int> entry)
                : data(std::move(entry)) {}

            explicit Liara_SettingStorage(Liara_FastSettingEntry<uint32_t> entry)
                : data(std::move(entry)) {}

            explicit Liara_SettingStorage(Liara_FastSettingEntry<float> entry)
                : data(std::move(entry)) {}

            explicit Liara_SettingStorage(Liara_FastSettingEntry<double> entry)
                : data(std::move(entry)) {}

            explicit Liara_SettingStorage(Liara_FlexibleSettingEntry entry)
                : data(std::move(entry)) {}

            Liara_SettingStorage(std::any defaultValue,
                                 const size_t typeHash,
                                 const SettingFlags flags = SettingFlags::DEFAULT)
                : data(Liara_FlexibleSettingEntry(std::move(defaultValue), typeHash, flags)) {}

            template <typename T> [[nodiscard]] bool HoldsType() const;
        };

    public:
        explicit Liara_SettingsManager(const ApplicationInfo& appInfo);
        ~Liara_SettingsManager() = default;

        Liara_SettingsManager(const Liara_SettingsManager&) = delete;
        Liara_SettingsManager& operator=(const Liara_SettingsManager&) = delete;
        Liara_SettingsManager(Liara_SettingsManager&&) = delete;
        Liara_SettingsManager& operator=(Liara_SettingsManager&&) = delete;

        template <typename T>
        void RegisterSetting(std::string_view name,
                             T&& defaultValue,
                             SettingFlags flags = SettingFlags::DEFAULT,
                             bool overwrite = false);

        template <typename T> [[nodiscard]] T Get(std::string_view name) const;

        template <typename T> bool Set(std::string_view name, const T& value);

        template <typename T> void Subscribe(std::string_view name, std::function<void(const T&)> callback);

        // Helpers pour les types courants (interface simplifiée)
        [[nodiscard]] bool GetBool(const std::string_view name) const { return Get<bool>(name); }
        [[nodiscard]] int GetInt(const std::string_view name) const { return Get<int>(name); }
        [[nodiscard]] uint32_t GetUInt(const std::string_view name) const { return Get<uint32_t>(name); }
        [[nodiscard]] float GetFloat(const std::string_view name) const { return Get<float>(name); }
        [[nodiscard]] std::string GetString(const std::string_view name) const { return Get<std::string>(name); }

        void SetBool(const std::string_view name, const bool value) { Set(name, value); }
        void SetInt(const std::string_view name, const int value) { Set(name, value); }
        void SetUInt(const std::string_view name, const uint32_t value) { Set(name, value); }
        void SetFloat(const std::string_view name, const float value) { Set(name, value); }
        void SetString(const std::string_view name, const std::string& value) { Set(name, value); }

        // Utility pour debug/introspection
        std::vector<std::string> GetAllSettingNames() const;

        template <typename T> bool HasSetting(std::string_view name) const;

        bool SaveToFile(const std::string& filename, bool overwrite = true) const;
        bool LoadFromFile(const std::string& filename);

    private:
        template <typename Entry>
        bool SerializeFastEntry(std::ofstream& file, const std::string& name, const Entry& entry) const;
        static bool
        SerializeFlexibleEntry(std::ofstream& file, const std::string& name, const Liara_FlexibleSettingEntry& entry);

        bool DeserializeSetting(const std::string& key, const std::string& value);
        template <typename Entry> bool DeserializeFastEntry(Entry& entry, const std::string& value);
        static bool DeserializeFlexibleEntry(const Liara_FlexibleSettingEntry& entry, const std::string& value);
        static bool IsSerializable(const Liara_SettingStorage& storage);

        static std::string Trim(const std::string& str);

        mutable std::shared_mutex m_Mutex;
        std::unordered_map<std::string, Liara_SettingStorage> m_Settings;
    };
}

#include "Liara_SettingsManager.tpp"  // IWYU pragma: keep
