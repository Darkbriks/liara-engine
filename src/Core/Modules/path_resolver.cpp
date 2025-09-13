#ifdef LIARA_MODULES_ENABLED
module;
#endif

#include "Core/Logging/LogMacros.h"

#include <filesystem>

#ifndef LIARA_MODULES_ENABLED

    #include <Liara/PathResolver.h>

#else

    #if defined(_WIN32) || defined(_WIN64)
export module liara.core.path_resolver;
    #else
module liara.core.path_resolver;
    #endif

#endif

namespace Liara::Core
{

    PathResolver::PathCache& PathResolver::GetCache() noexcept {
        static PathCache cache;
        return cache;
    }

    std::filesystem::path PathResolver::ResolveAssetPath(const std::string_view relativePath) {
        const auto& cache = GetCache();
        if (!cache.initialized) [[unlikely]] { InitializeCache(); }

        auto fullPath = cache.assets / relativePath;

        if (!std::filesystem::exists(fullPath)) [[unlikely]] {
            LIARA_THROW_RUNTIME_ERROR(LogCore, "Asset file not found: {}", fullPath.string());
        }

        return fullPath;
    }

    std::filesystem::path PathResolver::ResolveShaderPath(const std::string_view shaderName) {
        const auto& cache = GetCache();
        if (!cache.initialized) [[unlikely]] { InitializeCache(); }

        auto fullPath = cache.shaders / shaderName;

        if (!std::filesystem::exists(fullPath)) [[unlikely]] {
            LIARA_THROW_RUNTIME_ERROR(LogCore, "Shader file not found: {}", fullPath.string());
        }

        return fullPath;
    }

    std::filesystem::path PathResolver::ResolvePathUnchecked(const std::string_view relativePath,
                                                             const bool isShader) noexcept {
        const auto& cache = GetCache();
        if (!cache.initialized) [[unlikely]] { InitializeCache(); }

        return isShader ? cache.shaders / relativePath : cache.assets / relativePath;
    }

    const std::filesystem::path& PathResolver::GetAssetsDirectory() noexcept {
        auto& cache = GetCache();
        if (!cache.initialized) [[unlikely]] { InitializeCache(); }
        return cache.assets;
    }

    const std::filesystem::path& PathResolver::GetShadersDirectory() noexcept {
        auto& cache = GetCache();
        if (!cache.initialized) [[unlikely]] { InitializeCache(); }
        return cache.shaders;
    }

    PathResolver::Environment PathResolver::GetEnvironment() noexcept {
        const auto& cache = GetCache();
        if (!cache.initialized) [[unlikely]] { InitializeCache(); }
        return cache.environment;
    }

    void PathResolver::Reset() noexcept {
        auto& cache = GetCache();
        cache.initialized = false;
        cache.environment = Environment::Unknown;
    }

    void PathResolver::InitializeCache() noexcept {
        auto& [root, assets, shaders, environment, initialized] = GetCache();

        try {
            environment = DetectEnvironment();
            root = DetermineRootPath(environment);

            switch (environment) {
                case Environment::AppImage:
                    assets = root / "share" / "liara-engine" / "assets";
                    shaders = root / "share" / "liara-engine" / "shaders";
                    break;

                case Environment::Development:
                    assets = root / "app/assets";
                    shaders = root / "app/shaders";
                    break;

                case Environment::Standalone:
                default:
                    assets = root / "assets";
                    shaders = root / "shaders";
                    break;
            }

            initialized = true;
        }
        catch (...) {
            // Fallback en cas d'erreur : utiliser le répertoire courant
            root = std::filesystem::current_path();
            assets = root / "assets";
            shaders = root / "shaders";
            environment = Environment::Unknown;
            initialized = true;
        }
    }

    PathResolver::Environment PathResolver::DetectEnvironment() noexcept {
        // 1. Vérifier AppImage via variables d'environnement
        if (!GetEnvVar("APPDIR").empty() && !GetEnvVar("APPIMAGE").empty()) { return Environment::AppImage; }

        // 2. Vérifier AppImage via nom d'exécutable
        try {
            const auto execPath = GetExecutablePath();

            if (const auto execName = execPath.filename().string();
                execName == "AppRun" || execName.find(".AppImage") != std::string::npos) {
                return Environment::AppImage;
            }

            // 3. Détecter environnement de développement
            if (const auto execDir = execPath.parent_path();
                std::filesystem::exists(execDir / ".." / "CMakeCache.txt")) {
                return Environment::Development;
            }
        }
        catch (...) {
            LIARA_LOG_WARNING(LogCore, "Failed to detect execution environment");
        }

        return Environment::Standalone;
    }

    std::filesystem::path PathResolver::GetExecutablePath() {
#ifdef _WIN32
        wchar_t buffer[MAX_PATH];
        DWORD size = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        if (size == 0 || size == MAX_PATH) {
            LIARA_THROW_RUNTIME_ERROR(LogCore, "Failed to get executable path on Windows");
        }
        return std::filesystem::path(buffer);

#elif defined(__linux__)
        char buffer[PATH_MAX];
        const ssize_t size = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (size == -1) { LIARA_THROW_RUNTIME_ERROR(LogCore, "Failed to get executable path on Linux"); }
        buffer[size] = '\0';
        return {buffer};

#else
    #error "Unsupported platform for PathResolver::GetExecutablePath()"
#endif
    }

    std::filesystem::path PathResolver::DetermineRootPath(const Environment env) {
        switch (env) {
            case Environment::AppImage: {
                if (const auto appDir = GetEnvVar("APPDIR"); !appDir.empty()) { return appDir; }
                // Fallback : essayer de déduire depuis l'exécutable
                const auto execPath = GetExecutablePath();
                return execPath.parent_path().parent_path();  // Remonter de usr/bin vers racine
            }

            case Environment::Development: {
                const auto execPath = GetExecutablePath();
                return execPath.parent_path().parent_path();  // Remonter du build dir vers projet
            }

            case Environment::Standalone:
            default: {
                const auto execPath = GetExecutablePath();
                return execPath.parent_path();  // Répertoire de l'exécutable
            }
        }
    }

    std::string PathResolver::GetEnvVar(const char* name) noexcept {
#ifdef _WIN32
        char* buffer = nullptr;
        size_t len = 0;
        if (_dupenv_s(&buffer, &len, name) == 0 && buffer != nullptr) {
            std::string value(buffer);
            free(buffer);
            return value;
        }
        return {};
#else
        if (const char* val = std::getenv(name)) { return {val}; }
        return {};
#endif
    }
}