if(NOT SHADER_FILES OR NOT EMBED_HEADER OR NOT EMBED_SOURCE)
    message(FATAL_ERROR "Missing required variables: SHADER_FILES, EMBED_HEADER, EMBED_SOURCE")
endif()

string(REPLACE " " ";" SHADER_LIST "${SHADER_FILES}")

set(HEADER_CONTENT "#pragma once\n")
set(HEADER_CONTENT "${HEADER_CONTENT}#include <cstdint>\n")
set(HEADER_CONTENT "${HEADER_CONTENT}#include <span>\n")
set(HEADER_CONTENT "${HEADER_CONTENT}#include <string_view>\n")
set(HEADER_CONTENT "${HEADER_CONTENT}#include <unordered_map>\n\n")
set(HEADER_CONTENT "${HEADER_CONTENT}namespace Liara::EmbeddedShaders {\n\n")

set(SOURCE_CONTENT "#include \"embedded_shaders.h\"\n\n")
set(SOURCE_CONTENT "${SOURCE_CONTENT}namespace Liara::EmbeddedShaders {\n\n")

set(PROCESSED_COUNT 0)
set(SHADER_MAP_ENTRIES "")

foreach(SHADER_FILE ${SHADER_LIST})
    if(EXISTS "${SHADER_FILE}")
        get_filename_component(SHADER_NAME "${SHADER_FILE}" NAME)
        string(REPLACE "." "_" SHADER_VAR "${SHADER_NAME}")

        file(READ "${SHADER_FILE}" SHADER_CONTENT HEX)

        if(SHADER_CONTENT)
            string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," SHADER_ARRAY "${SHADER_CONTENT}")
            string(REGEX REPLACE ",$" "" SHADER_ARRAY "${SHADER_ARRAY}")

            # Add individual shader data declarations to header
            set(HEADER_CONTENT "${HEADER_CONTENT}    extern const uint8_t ${SHADER_VAR}_data[];\n")
            set(HEADER_CONTENT "${HEADER_CONTENT}    extern const std::span<const uint8_t> ${SHADER_VAR};\n")
            set(HEADER_CONTENT "${HEADER_CONTENT}    extern const size_t ${SHADER_VAR}_size;\n\n")

            # Add individual shader data definitions to source
            set(SOURCE_CONTENT "${SOURCE_CONTENT}    const uint8_t ${SHADER_VAR}_data[] = {${SHADER_ARRAY}};\n")
            set(SOURCE_CONTENT "${SOURCE_CONTENT}    const std::span<const uint8_t> ${SHADER_VAR}{${SHADER_VAR}_data, sizeof(${SHADER_VAR}_data)};\n")
            set(SOURCE_CONTENT "${SOURCE_CONTENT}    const size_t ${SHADER_VAR}_size = sizeof(${SHADER_VAR}_data);\n\n")

            # Build map entry for automatic lookup
            set(SHADER_MAP_ENTRIES "${SHADER_MAP_ENTRIES}            {\"${SHADER_NAME}\", ${SHADER_VAR}},\n")

            math(EXPR PROCESSED_COUNT "${PROCESSED_COUNT} + 1")
            message(STATUS "Embedded shader: ${SHADER_NAME} (${SHADER_VAR})")
        else()
            message(WARNING "Empty shader file: ${SHADER_FILE}")
        endif()
    else()
        message(WARNING "Shader file not found: ${SHADER_FILE}")
    endif()
endforeach()

# Add lookup function declaration to header
set(HEADER_CONTENT "${HEADER_CONTENT}    /**\n")
set(HEADER_CONTENT "${HEADER_CONTENT}     * @brief Get embedded shader data by name\n")
set(HEADER_CONTENT "${HEADER_CONTENT}     * @param shaderName Name of the shader file (e.g., \"SimpleShader.vert.spv\")\n")
set(HEADER_CONTENT "${HEADER_CONTENT}     * @return Span containing shader bytecode, or empty span if not found\n")
set(HEADER_CONTENT "${HEADER_CONTENT}     */\n")
set(HEADER_CONTENT "${HEADER_CONTENT}    [[nodiscard]] std::span<const uint8_t> GetShader(std::string_view shaderName) noexcept;\n\n")

set(HEADER_CONTENT "${HEADER_CONTENT}    /**\n")
set(HEADER_CONTENT "${HEADER_CONTENT}     * @brief Check if a shader is available\n")
set(HEADER_CONTENT "${HEADER_CONTENT}     * @param shaderName Name of the shader file\n")
set(HEADER_CONTENT "${HEADER_CONTENT}     * @return true if shader is embedded, false otherwise\n")
set(HEADER_CONTENT "${HEADER_CONTENT}     */\n")
set(HEADER_CONTENT "${HEADER_CONTENT}    [[nodiscard]] bool HasShader(std::string_view shaderName) noexcept;\n\n")

set(HEADER_CONTENT "${HEADER_CONTENT}    /**\n")
set(HEADER_CONTENT "${HEADER_CONTENT}     * @brief Get number of embedded shaders\n")
set(HEADER_CONTENT "${HEADER_CONTENT}     * @return Number of available shaders\n")
set(HEADER_CONTENT "${HEADER_CONTENT}     */\n")
set(HEADER_CONTENT "${HEADER_CONTENT}    [[nodiscard]] constexpr size_t GetShaderCount() noexcept { return ${PROCESSED_COUNT}; }\n\n")

# Add lookup function implementation to source
set(SOURCE_CONTENT "${SOURCE_CONTENT}    std::span<const uint8_t> GetShader(const std::string_view shaderName) noexcept {\n")
set(SOURCE_CONTENT "${SOURCE_CONTENT}        static const std::unordered_map<std::string_view, std::span<const uint8_t>> shaderMap = {\n")
set(SOURCE_CONTENT "${SOURCE_CONTENT}${SHADER_MAP_ENTRIES}")
set(SOURCE_CONTENT "${SOURCE_CONTENT}        };\n\n")
set(SOURCE_CONTENT "${SOURCE_CONTENT}        if (const auto it = shaderMap.find(shaderName); it != shaderMap.end()) {\n")
set(SOURCE_CONTENT "${SOURCE_CONTENT}            return it->second;\n")
set(SOURCE_CONTENT "${SOURCE_CONTENT}        }\n")
set(SOURCE_CONTENT "${SOURCE_CONTENT}        return {}; // Empty span if not found\n")
set(SOURCE_CONTENT "${SOURCE_CONTENT}    }\n\n")

set(SOURCE_CONTENT "${SOURCE_CONTENT}    bool HasShader(const std::string_view shaderName) noexcept {\n")
set(SOURCE_CONTENT "${SOURCE_CONTENT}        return !GetShader(shaderName).empty();\n")
set(SOURCE_CONTENT "${SOURCE_CONTENT}    }\n\n")

set(HEADER_CONTENT "${HEADER_CONTENT}}\n")
set(SOURCE_CONTENT "${SOURCE_CONTENT}}\n")

# Write files
file(WRITE "${EMBED_HEADER}" "${HEADER_CONTENT}")
file(WRITE "${EMBED_SOURCE}" "${SOURCE_CONTENT}")

message(STATUS "Successfully embedded ${PROCESSED_COUNT} shaders")