module;

#include "Core/Logging/LogMacros.h"

#include <filesystem>
#include <string_view>
#include <string>
#include <cstdint>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#elif defined(__linux__)
#include <unistd.h>
#include <climits>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#endif

export module liara.core.utils.path_resolver;

export namespace Liara::Core {

    /// Types d'environnement d'exécution
    enum class Environment : std::uint8_t {
        Development,    ///< Environnement de développement (build directory)
        AppImage,       ///< Application empaquetée en AppImage
        Standalone,     ///< Exécutable standalone classique
        Unknown         ///< Environnement non déterminé
    };

    /**
     * @brief Résolveur de chemins cross-platform avec support AppImage
     *
     * Module C++20 optimisé pour la performance et l'encapsulation.
     * Gère automatiquement la résolution des chemins d'assets selon l'environnement.
     */
    class PathResolver {
    public:
        /**
         * @brief Résout le chemin vers un asset
         * @param relativePath Chemin relatif de l'asset
         * @return Chemin absolu vers l'asset
         * @throws std::runtime_error si l'asset n'existe pas
         */
        [[nodiscard]] static std::filesystem::path ResolveAssetPath(const std::string_view relativePath) {
            const auto& cache = GetCache();
            if (!cache.initialized) [[unlikely]] { InitializeCache(); }

            auto fullPath = cache.assets / relativePath;

            if (!std::filesystem::exists(fullPath)) [[unlikely]] {
                const std::string pathStr = fullPath.string();
                LIARA_THROW_RUNTIME_ERROR(LogCore, "Asset file not found: {}", pathStr);
            }

            return fullPath;
        }

        /**
         * @brief Résout le chemin vers un shader
         * @param shaderName Nom du shader
         * @return Chemin absolu vers le shader
         * @throws std::runtime_error si le shader n'existe pas
         */
        [[nodiscard]] static std::filesystem::path ResolveShaderPath(std::string_view shaderName) {
            const auto& cache = GetCache();
            if (!cache.initialized) [[unlikely]] { InitializeCache(); }

            auto fullPath = cache.shaders / shaderName;

            if (!std::filesystem::exists(fullPath)) [[unlikely]] {
                const std::string pathStr = fullPath.string();
                LIARA_THROW_RUNTIME_ERROR(LogCore, "Shader file not found: {}", pathStr);
            }

            return fullPath;
        }

        /**
         * @brief Résout un chemin sans vérification d'existence (performance)
         * @param relativePath Chemin relatif
         * @param isShader true si c'est un shader, false si c'est un asset
         * @return Chemin absolu (non vérifié)
         */
        [[nodiscard]] static std::filesystem::path ResolvePathUnchecked(std::string_view relativePath, bool isShader = false) noexcept {
            const auto& cache = GetCache();
            if (!cache.initialized) [[unlikely]] { InitializeCache(); }

            return isShader ? cache.shaders / relativePath : cache.assets / relativePath;
        }

        /**
         * @brief Obtient le répertoire racine des assets
         * @return Chemin vers le répertoire assets
         */
        [[nodiscard]] static const std::filesystem::path& GetAssetsDirectory() noexcept {
            auto& cache = GetCache();
            if (!cache.initialized) [[unlikely]] { InitializeCache(); }
            return cache.assets;
        }

        /**
         * @brief Obtient le répertoire des shaders
         * @return Chemin vers le répertoire shaders
         */
        [[nodiscard]] static const std::filesystem::path& GetShadersDirectory() noexcept {
            auto& cache = GetCache();
            if (!cache.initialized) [[unlikely]] { InitializeCache(); }
            return cache.shaders;
        }

        /**
         * @brief Obtient l'environnement d'exécution détecté
         * @return Type d'environnement
         */
        [[nodiscard]] static Environment GetEnvironment() noexcept {
            const auto& cache = GetCache();
            if (!cache.initialized) [[unlikely]] { InitializeCache(); }
            return cache.environment;
        }

        /**
         * @brief Vérifie si l'application s'exécute dans une AppImage
         * @return true si dans une AppImage
         */
        [[nodiscard]] static bool IsAppImage() noexcept {
            return GetEnvironment() == Environment::AppImage;
        }

        /**
         * @brief Force la réinitialisation des chemins (pour tests)
         */
        static void Reset() noexcept {
            auto& cache = GetCache();
            cache.initialized = false;
            cache.environment = Environment::Unknown;
        }

    private:
        /// Cache des chemins résolus pour la performance
        struct PathCache {
            std::filesystem::path root;
            std::filesystem::path assets;
            std::filesystem::path shaders;
            Environment environment{Environment::Unknown};
            bool initialized{false};
        };

        static PathCache& GetCache() noexcept {
            static PathCache cache;
            return cache;
        }

        static void InitializeCache() noexcept {
            auto& cache = GetCache();

            try {
                cache.environment = DetectEnvironment();
                cache.root = DetermineRootPath(cache.environment);

                switch (cache.environment) {
                    case Environment::AppImage:
                        cache.assets = cache.root / "share" / "liara-engine" / "assets";
                        cache.shaders = cache.root / "share" / "liara-engine" / "shaders";
                        break;

                    case Environment::Development:
                        cache.assets = cache.root / "app" / "assets";
                        cache.shaders = cache.root / "app" / "shaders";
                        break;

                    case Environment::Standalone:
                    default:
                        cache.assets = cache.root / "assets";
                        cache.shaders = cache.root / "shaders";
                        break;
                }

                cache.initialized = true;
            }
            catch (...) {
                // Fallback en cas d'erreur : utiliser le répertoire courant
                cache.root = std::filesystem::current_path();
                cache.assets = cache.root / "assets";
                cache.shaders = cache.root / "shaders";
                cache.environment = Environment::Unknown;
                cache.initialized = true;

                LIARA_LOG_WARNING(LogCore, "PathResolver initialization failed, using fallback paths");
            }
        }

        static Environment DetectEnvironment() noexcept {
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

        static std::filesystem::path GetExecutablePath() {
            #ifdef _WIN32
            wchar_t buffer[MAX_PATH];
            const DWORD size = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
            if (size == 0 || size == MAX_PATH) {
                LIARA_THROW_RUNTIME_ERROR(LogCore, "Failed to get executable path on Windows");
            }
            return std::filesystem::path(buffer);

            #elif defined(__linux__)
            char buffer[PATH_MAX];
            const ssize_t size = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
            if (size == -1) { LIARA_THROW_RUNTIME_ERROR(LogCore, "Failed to get executable path on Linux"); }
            buffer[size] = '\0';
            return std::filesystem::path(buffer);

            #else
            #error "Unsupported platform for PathResolver::GetExecutablePath()"
            #endif
        }

        static std::filesystem::path DetermineRootPath(Environment env) {
            switch (env) {
                case Environment::AppImage: {
                    if (const auto appDir = GetEnvVar("APPDIR"); !appDir.empty()) { return std::filesystem::path(appDir); }
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

        [[nodiscard]] static std::string GetEnvVar(const char* name) noexcept {
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
    };

}