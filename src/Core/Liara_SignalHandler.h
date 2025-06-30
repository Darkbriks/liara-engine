#pragma once

#include <atomic>
#include <functional>

namespace Liara::Core
{
    /**
     * @brief Thread-safe signal handler for graceful application shutdown
     */
    class Liara_SignalHandler
    {
    public:
        using ShutdownCallback = std::function<void()>;

        /**
         * @brief Initialize signal handling
         * @param callback Optional callback to execute on shutdown signal
         * @return true if initialization successful
         */
        static bool Initialize(ShutdownCallback callback = nullptr) noexcept;

        /**
         * @brief Cleanup signal handling
         */
        static void Cleanup() noexcept;

        /**
         * @brief Check if shutdown was requested via signal
         * @return true if application should exit
         */
        [[nodiscard]] static bool ShouldExit() noexcept { return s_shutdownRequested.load(std::memory_order_acquire); }

    private:
        static void SignalHandler(int signal) noexcept;

        static inline std::atomic<bool> s_shutdownRequested{false};
        static inline ShutdownCallback s_callback{nullptr};
        static inline bool s_initialized{false};
    };
}
