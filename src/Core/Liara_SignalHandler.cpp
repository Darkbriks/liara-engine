#include "Liara_SignalHandler.h"

#include <iostream>

#include "Logging/LogMacros.h"

#ifdef _WIN32
    #include <signal.h>
    #include <windows.h>
#else
    #include <csignal>
    #include <unistd.h>
#endif

namespace Liara::Core
{
    bool Liara_SignalHandler::Initialize(ShutdownCallback callback) noexcept {
        if (s_initialized) { return true; }
        s_callback = std::move(callback);

#ifdef _WIN32
        // Windows: Handle Ctrl+C and Ctrl+Break
        if (!SetConsoleCtrlHandler(
                [](DWORD dwCtrlType) -> BOOL {
                    switch (dwCtrlType) {
                        case CTRL_C_EVENT:
                        case CTRL_BREAK_EVENT:
                        case CTRL_CLOSE_EVENT: SignalHandler(SIGINT); return TRUE;
                        default: return FALSE;
                    }
                },
                TRUE)) {
            LIARA_LOG_ERROR(LogCore, "Failed to set Windows console handler");
            return false;
        }
#else
        // POSIX: Handle SIGINT and SIGTERM
        struct sigaction sa{};
        sa.sa_handler = SignalHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;

        if (sigaction(SIGINT, &sa, nullptr) == -1) {
            LIARA_LOG_ERROR(LogCore, "Failed to set SIGINT handler");
            return false;
        }

        if (sigaction(SIGTERM, &sa, nullptr) == -1) {
            LIARA_LOG_ERROR(LogCore, "Failed to set SIGTERM handler");
            return false;
        }
#endif

        s_initialized = true;
        return true;
    }

    void Liara_SignalHandler::Cleanup() noexcept {
        if (!s_initialized) { return; }

#ifdef _WIN32
        SetConsoleCtrlHandler(nullptr, FALSE);
#else
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
#endif

        s_initialized = false;
        s_callback = nullptr;
    }

    void Liara_SignalHandler::SignalHandler(int /*signal*/) noexcept {
        s_shutdownRequested.store(true, std::memory_order_release);

        if (s_callback) {
            try {
                s_callback();
            }
            catch (const std::exception& e) {
                LIARA_LOG_ERROR(LogCore, "Error in shutdown callback: {}", e.what());
            }
            catch (...) {
                LIARA_LOG_ERROR(LogCore, "Unknown error in shutdown callback");
            }
        }
    }
}