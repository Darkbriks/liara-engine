#pragma once

#include <functional>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace Liara::Core
{
    class BadResultAccess final : public std::runtime_error
    {
    public:
        explicit BadResultAccess(const std::string& msg)
            : std::runtime_error(msg) {}
    };

    /**
     * @brief Class simulating std::expected (C++23) for C++20.
     *
     * @tparam T Type of the value in case of success
     * @tparam E Type of the error in case of failure
     */
    template <typename T, typename E = int> class Result
    {
    private:
        std::optional<T> m_value;
        std::optional<E> m_error;

    public:
        using value_type = T;
        using error_type = E;

        template <typename U = T>
            requires std::constructible_from<T, U&&>
        explicit Result(U&& value) noexcept(std::is_nothrow_constructible_v<T, U&&>)
            : m_value(std::forward<U>(value))
            , m_error() {}

        template <typename F = E>
            requires std::constructible_from<E, F&&>
        explicit Result(F&& error) noexcept(std::is_nothrow_constructible_v<E, F&&>)
            : m_value()
            , m_error(std::forward<F>(error)) {}

        Result() = delete;
        ~Result() = default;

        Result(const Result&) = default;
        Result(Result&&) = default;
        Result& operator=(const Result&) = default;
        Result& operator=(Result&&) = default;

        /**
         * @brief Check if the Result contains a value (success)
         */
        [[nodiscard]] constexpr bool HasValue() const noexcept { return m_value.has_value(); }

        /**
         * @brief Check if the Result contains an error (failure)
         */
        [[nodiscard]] constexpr bool HasError() const noexcept { return m_error.has_value(); }

        /**
         * @brief Check if the Result has a value or an error
         */
        [[nodiscard]] constexpr explicit operator bool() const noexcept { return HasValue(); }

        /**
         * @brief Direct access to the value
         * @throw BadResultAccess if no value
         */
        [[nodiscard]] constexpr const T& Value() const& {
            if (!HasValue()) { throw BadResultAccess("Attempting to access value of failed Result"); }
            return m_value.value();
        }

        /**
         * @brief Access to the value
         * @throws BadResultAccess if no value
         */
        [[nodiscard]] constexpr T& Value() & {
            if (!HasValue()) { throw BadResultAccess("Attempting to access value of failed Result"); }
            return m_value.value();
        }

        /**
         * @brief Access to the value
         * @throws BadResultAccess if no value
         */
        [[nodiscard]] constexpr T&& Value() && {
            if (!HasValue()) { throw BadResultAccess("Attempting to access value of failed Result"); }
            return std::move(m_value.value());
        }

        /**
         * @brief Access to the error
         * @throws BadResultAccess if no error
         */
        [[nodiscard]] constexpr const E& Error() const& {
            if (!HasError()) { throw BadResultAccess("Attempting to access error of successful Result"); }
            return m_error.value();
        }

        /**
         * @brief Access to the error
         * @throws BadResultAccess if no error
         */
        [[nodiscard]] constexpr E&& Error() && {
            if (!HasError()) { throw BadResultAccess("Attempting to access error of successful Result"); }
            return std::move(m_error.value());
        }

        [[nodiscard]] constexpr const T& operator*() const& noexcept { return m_value.value(); }

        [[nodiscard]] constexpr T& operator*() & noexcept { return m_value.value(); }

        [[nodiscard]] constexpr T&& operator*() && noexcept { return std::move(m_value.value()); }

        [[nodiscard]] constexpr const T* operator->() const noexcept { return &m_value.value(); }

        [[nodiscard]] constexpr T* operator->() noexcept { return &m_value.value(); }

        template <typename U> [[nodiscard]] constexpr T ValueOr(U&& defaultValue) const& {
            return HasValue() ? Value() : static_cast<T>(std::forward<U>(defaultValue));
        }

        template <typename U> [[nodiscard]] constexpr T ValueOr(U&& defaultValue) && {
            return HasValue() ? std::move(Value()) : static_cast<T>(std::forward<U>(defaultValue));
        }
    };

    template <typename T, typename E = void> [[nodiscard]] constexpr auto Ok(T&& value) -> Result<std::decay_t<T>, E> {
        return Result<std::decay_t<T>, E>(std::forward<T>(value));
    }

    template <typename E, typename T = void> [[nodiscard]] constexpr auto Err(E&& error) -> Result<T, std::decay_t<E>> {
        return Result<T, std::decay_t<E>>(std::forward<E>(error));
    }

    template <typename T, typename E>
    [[nodiscard]] constexpr bool operator==(const Result<T, E>& lhs, const Result<T, E>& rhs) {
        if (lhs.HasValue() && rhs.HasValue()) { return lhs.Value() == rhs.Value(); }
        if (lhs.has_error() && rhs.has_error()) { return lhs.Error() == rhs.Error(); }
        return false;
    }

    template <typename T, typename E>
    [[nodiscard]] constexpr bool operator!=(const Result<T, E>& lhs, const Result<T, E>& rhs) {
        return !(lhs == rhs);
    }
}