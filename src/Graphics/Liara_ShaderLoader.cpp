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

    ShaderLoader::SpanResult ShaderLoader::LoadShaderSpan(const std::string_view shaderName) {
#ifdef LIARA_EMBED_SHADERS
        return LoadEmbeddedShader(shaderName);
#else
        return Core::Err<ShaderLoadError, ShaderSpan>(ShaderLoadError::EmbeddedNotAvailable);
#endif
    }

#ifdef LIARA_EMBED_SHADERS
    ShaderLoader::SpanResult ShaderLoader::LoadEmbeddedShader(const std::string_view shaderName) {
        const std::string identifier = ShaderNameToIdentifier(shaderName);

        // TODO: Generate this list automatically from embedded shaders
        if (identifier == "SimpleShader_vert_spv") {
            const auto& data = EmbeddedShaders::SimpleShader_vert_spv;
            return Core::Ok<ShaderSpan, ShaderLoadError>(
                ShaderSpan(reinterpret_cast<const uint32_t*>(data.data()), data.size() / sizeof(uint32_t)));
        }
        if (identifier == "SimpleShader_frag_spv") {
            const auto& data = EmbeddedShaders::SimpleShader_frag_spv;
            return Core::Ok<ShaderSpan, ShaderLoadError>(
                ShaderSpan(reinterpret_cast<const uint32_t*>(data.data()), data.size() / sizeof(uint32_t)));
        }
        if (identifier == "PointLight_vert_spv") {
            const auto& data = EmbeddedShaders::PointLight_vert_spv;
            return Core::Ok<ShaderSpan, ShaderLoadError>(
                ShaderSpan(reinterpret_cast<const uint32_t*>(data.data()), data.size() / sizeof(uint32_t)));
        }
        if (identifier == "PointLight_frag_spv") {
            const auto& data = EmbeddedShaders::PointLight_frag_spv;
            return Core::Ok<ShaderSpan, ShaderLoadError>(
                ShaderSpan(reinterpret_cast<const uint32_t*>(data.data()), data.size() / sizeof(uint32_t)));
        }

        return Core::Err<ShaderLoadError, ShaderSpan>(ShaderLoadError::FileNotFound);
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