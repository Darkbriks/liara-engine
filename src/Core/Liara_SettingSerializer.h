#pragma once
#include <charconv>
#include <format>
#include <string>
#include <type_traits>

namespace Liara::Core {
    template<typename T>
    struct SettingSerializer {
        static_assert(!sizeof(T), "No serializer available for this type. Specialize SettingSerializer<T>");
    };

    // Spécialisations pour les types de base
    template<>
    struct SettingSerializer<bool> {
        static std::string serialize(const bool value) {
            return value ? "true" : "false";
        }

        static bool deserialize(const std::string_view str, bool& out) {
            if (str == "true") { out = true; return true; }
            if (str == "false") { out = false; return true; }
            return false;
        }
    };

    template<std::integral T>
    struct SettingSerializer<T> {
        static std::string serialize(T value) {
            return std::to_string(value);
        }

        static bool deserialize(std::string_view str, T& out) {
            auto result = std::from_chars(str.data(), str.data() + str.size(), out);
            return result.ec == std::errc{};
        }
    };

    template<std::floating_point T>
    struct SettingSerializer<T> {
        static std::string serialize(T value) {
            return std::format("{:.6f}", value); // 6 décimales de précision
        }

        static bool deserialize(std::string_view str, T& out) {
            auto result = std::from_chars(str.data(), str.data() + str.size(), out);
            return result.ec == std::errc{};
        }
    };

    template<>
    struct SettingSerializer<std::string> {
        static std::string serialize(const std::string& value) {
            // Échappement simple pour newlines et quotes
            std::string result;
            result.reserve(value.size() + 10);
            result += '"';

            for (const char c : value) {
                switch (c) {
                    case '"': result += "\\\""; break;
                    case '\\': result += "\\\\"; break;
                    case '\n': result += "\\n"; break;
                    case '\r': result += "\\r"; break;
                    case '\t': result += "\\t"; break;
                    default: result += c; break;
                }
            }

            result += '"';
            return result;
        }

        static bool deserialize(std::string_view str, std::string& out) {
            if (str.size() < 2 || str.front() != '"' || str.back() != '"') {
                return false;
            }

            str = str.substr(1, str.size() - 2); // Enlever les quotes
            out.clear();
            out.reserve(str.size());

            for (size_t i = 0; i < str.size(); ++i) {
                if (str[i] == '\\' && i + 1 < str.size()) {
                    switch (str[i + 1]) {
                        case '"': out += '"'; break;
                        case '\\': out += '\\'; break;
                        case 'n': out += '\n'; break;
                        case 'r': out += '\r'; break;
                        case 't': out += '\t'; break;
                        default: out += str[i + 1]; break;
                    }
                    ++i; // Skip next char
                } else {
                    out += str[i];
                }
            }

            return true;
        }
    };

    // Support pour les enums
    template<typename T>
    requires std::is_enum_v<T>
    struct SettingSerializer<T> {
        static std::string serialize(T value) {
            return std::to_string(static_cast<std::underlying_type_t<T>>(value));
        }

        static bool deserialize(std::string_view str, T& out) {
            std::underlying_type_t<T> underlying;
            if (auto result = std::from_chars(str.data(), str.data() + str.size(), underlying); result.ec == std::errc{}) {
                out = static_cast<T>(underlying);
                return true;
            }
            return false;
        }
    };

    template<typename T>
    concept SerializableType = requires(T value, std::string_view str) {
        { SettingSerializer<T>::serialize(value) } -> std::convertible_to<std::string>;
        { SettingSerializer<T>::deserialize(str, value) } -> std::convertible_to<bool>;
    };

    // Interface pour la sérialisation des types complexes
    class ISettingSerializable {
    public:
        virtual ~ISettingSerializable() = default;
        [[nodiscard]] virtual std::string serialize() const = 0;
        virtual bool deserialize(std::string_view data) = 0;
    };
}
