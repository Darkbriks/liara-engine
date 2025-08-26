#include "Liara_ShaderLoader.h"

#include <algorithm>
#include <fstream>

#ifndef ENGINE_DIR
    #define ENGINE_DIR "./"
#endif

namespace Liara::Graphics
{
    ShaderLoader::LoadResult ShaderLoader::LoadShader(const std::string_view shaderName) {
#ifdef LIARA_EMBED_SHADERS
        if (auto spanResult = LoadEmbeddedShader(shaderName)) {
            const auto& span = spanResult.Value();
            return Core::Ok<ShaderData, ShaderLoadError>(ShaderData(span.begin(), span.end()));
        }
#endif

        const auto shaderPath = std::filesystem::path(ENGINE_DIR) / "shaders" / shaderName;
        return LoadShaderFromFile(shaderPath);
    }

#ifdef LIARA_EMBED_SHADERS
    ShaderLoader::SpanResult ShaderLoader::LoadShaderSpan(const std::string_view shaderName) {
        return LoadEmbeddedShader(shaderName);
    }

    bool ShaderLoader::HasEmbeddedShader(const std::string_view shaderName) noexcept {
        return EmbeddedShaders::HasShader(shaderName);
    }

    constexpr size_t ShaderLoader::GetEmbeddedShaderCount() noexcept { return EmbeddedShaders::GetShaderCount(); }

    ShaderLoader::SpanResult ShaderLoader::LoadEmbeddedShader(const std::string_view shaderName) {
        const auto embeddedData = EmbeddedShaders::GetShader(shaderName);

        if (embeddedData.empty()) { return Core::Err<ShaderLoadError, ShaderSpan>(ShaderLoadError::FileNotFound); }

        if (embeddedData.size() % sizeof(uint32_t) != 0) {
            return Core::Err<ShaderLoadError, ShaderSpan>(ShaderLoadError::InvalidFormat);
        }

        const auto* dataPtr = reinterpret_cast<const uint32_t*>(embeddedData.data());
        const size_t elementCount = embeddedData.size() / sizeof(uint32_t);

        return Core::Ok<ShaderSpan, ShaderLoadError>(ShaderSpan(dataPtr, elementCount));
    }
#else
    ShaderLoader::SpanResult ShaderLoader::LoadShaderSpan(const std::string_view) {
        return Core::Err<ShaderLoadError, ShaderSpan>(ShaderLoadError::EmbeddedNotAvailable);
    }
#endif

    ShaderLoader::LoadResult ShaderLoader::LoadShaderFromFile(const std::filesystem::path& shaderPath) {
        if (!std::filesystem::exists(shaderPath)) {
            return Core::Err<ShaderLoadError, ShaderData>(ShaderLoadError::FileNotFound);
        }

        std::ifstream file(shaderPath, std::ios::ate | std::ios::binary);
        if (!file.is_open()) { return Core::Err<ShaderLoadError, ShaderData>(ShaderLoadError::ReadError); }

        const size_t fileSize = file.tellg();
        if (fileSize % sizeof(uint32_t) != 0) {
            return Core::Err<ShaderLoadError, ShaderData>(ShaderLoadError::InvalidFormat);
        }

        ShaderData buffer(fileSize / sizeof(uint32_t));
        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(fileSize));

        if (!file.good()) { return Core::Err<ShaderLoadError, ShaderData>(ShaderLoadError::ReadError); }

        return Core::Ok<ShaderData, ShaderLoadError>(std::move(buffer));
    }

    std::string ShaderLoader::ShaderNameToIdentifier(const std::string_view shaderName) {
        std::string identifier{shaderName};
        std::ranges::replace(identifier, '.', '_');
        return identifier;
    }

    std::string ToString(const ShaderLoadError error) {
        switch (error) {
            case ShaderLoadError::FileNotFound: return "Shader file not found";
            case ShaderLoadError::ReadError: return "Failed to read shader file";
            case ShaderLoadError::InvalidFormat: return "Invalid shader format";
            case ShaderLoadError::EmbeddedNotAvailable: return "Embedded shader not available";
            default: return "Unknown error";
        }
    }

    std::vector<char> ConvertToCharVector(const std::vector<uint32_t>& data) {
        const auto* const charData = reinterpret_cast<const char*>(data.data());
        return {charData, charData + (data.size() * sizeof(uint32_t))};
    }
}