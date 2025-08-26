#pragma once

#include <Liara/Utils/Result.h>  // Simulate std::expected

#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#ifdef LIARA_EMBED_SHADERS
    #include "shaders/embedded_shaders.h"
#endif

namespace Liara::Graphics
{
    /**
     * @brief Enum that represents possible errors when loading shaders
     */
    enum class ShaderLoadError : uint8_t
    {
        NoError,
        FileNotFound,
        ReadError,
        InvalidFormat,
        EmbeddedNotAvailable
    };

    /**
     * @brief Class for shader loading
     *
     * Can load shaders either from embedded resources or from files
     */
    class ShaderLoader
    {
    public:
        using ShaderData = std::vector<uint32_t>;
        using ShaderSpan = std::span<const uint32_t>;
        using LoadResult = Core::Result<ShaderData, ShaderLoadError>;
        using SpanResult = Core::Result<ShaderSpan, ShaderLoadError>;

        /**
         * @brief Loads a shader from the specified name
         *
         * Try to load the shader from embedded resources first,
         * then from the file system if not found.
         */
        [[nodiscard]] static LoadResult LoadShader(std::string_view shaderName);

        /**
         * @brief Optmized version that returns a span instead of a vector,
         * to avoid unnecessary copies.
         */
#ifdef LIARA_EMBED_SHADERS
        [[nodiscard]] static SpanResult LoadShaderSpan(std::string_view shaderName);
#else
        [[nodiscard]] static SpanResult LoadShaderSpan(std::string_view);
#endif

        /**
         * @brief Check if a shader is available in embedded resources
         * @param shaderName Name of the shader (e.g., "SimpleShader.vert.spv")
         * @return true if shader is embedded, false otherwise
         */
#ifdef LIARA_EMBED_SHADERS
        [[nodiscard]] static bool HasEmbeddedShader(std::string_view shaderName) noexcept;
#else
        [[nodiscard]] static constexpr bool HasEmbeddedShader(std::string_view) noexcept { return false; }
#endif

        /**
         * @brief Get the number of embedded shaders
         * @return Number of available embedded shaders
         */
#ifdef LIARA_EMBED_SHADERS
        [[nodiscard]] static constexpr size_t GetEmbeddedShaderCount() noexcept;
#else
        [[nodiscard]] static constexpr size_t GetEmbeddedShaderCount() noexcept { return 0; }
#endif

    private:
#ifdef LIARA_EMBED_SHADERS
        [[nodiscard]] static SpanResult LoadEmbeddedShader(std::string_view shaderName);
#endif

        [[nodiscard]] static LoadResult LoadShaderFromFile(const std::filesystem::path& shaderPath);
        [[nodiscard]] static std::string ShaderNameToIdentifier(std::string_view shaderName);
    };

    /**
     * @brief Helper to convert ShaderLoadError to a human-readable string
     */
    [[nodiscard]] std::string ToString(ShaderLoadError error);

    /**
     * @brief Helper to convert a vector of uint32_t to a vector of char
     */
    [[nodiscard]] std::vector<char> ConvertToCharVector(const std::vector<uint32_t>& data);
}