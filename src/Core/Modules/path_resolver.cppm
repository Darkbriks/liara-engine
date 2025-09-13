module;

#include <filesystem>
#include <string_view>
#include <string>
#include <cstdint>

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

export module liara.core.path_resolver;

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
    [[nodiscard]] static std::filesystem::path ResolveAssetPath(std::string_view relativePath);

    /**
     * @brief Résout le chemin vers un shader
     * @param shaderName Nom du shader
     * @return Chemin absolu vers le shader
     * @throws std::runtime_error si le shader n'existe pas
     */
    [[nodiscard]] static std::filesystem::path ResolveShaderPath(std::string_view shaderName);

    /**
     * @brief Résout un chemin sans vérification d'existence (performance)
     * @param relativePath Chemin relatif
     * @param isShader true si c'est un shader, false si c'est un asset
     * @return Chemin absolu (non vérifié)
     */
    [[nodiscard]] static std::filesystem::path ResolvePathUnchecked(std::string_view relativePath, bool isShader = false) noexcept;

    /**
     * @brief Obtient le répertoire racine des assets
     * @return Chemin vers le répertoire assets
     */
    [[nodiscard]] static const std::filesystem::path& GetAssetsDirectory() noexcept;

    /**
     * @brief Obtient le répertoire des shaders
     * @return Chemin vers le répertoire shaders
     */
    [[nodiscard]] static const std::filesystem::path& GetShadersDirectory() noexcept;

    /**
     * @brief Obtient l'environnement d'exécution détecté
     * @return Type d'environnement
     */
    [[nodiscard]] static Environment GetEnvironment() noexcept;

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
    static void Reset() noexcept;

private:
    /// Cache des chemins résolus pour la performance
    struct PathCache {
        std::filesystem::path root;
        std::filesystem::path assets;
        std::filesystem::path shaders;
        Environment environment{Environment::Unknown};
        bool initialized{false};
    };

    static PathCache& GetCache() noexcept;
    static void InitializeCache() noexcept;
    static Environment DetectEnvironment() noexcept;
    static std::filesystem::path GetExecutablePath();
    static std::filesystem::path DetermineRootPath(Environment env);

    [[nodiscard]] static std::string GetEnvVar(const char* name) noexcept;
};

}