if(NOT SHADER_FILES OR NOT EMBED_HEADER OR NOT EMBED_SOURCE)
    message(FATAL_ERROR "Missing required variables: SHADER_FILES, EMBED_HEADER, EMBED_SOURCE")
endif()

string(REPLACE " " ";" SHADER_LIST "${SHADER_FILES}")

set(HEADER_CONTENT "#pragma once\n#include <cstdint>\n#include <span>\n\n")
set(HEADER_CONTENT "${HEADER_CONTENT}namespace Liara::EmbeddedShaders {\n\n")

set(SOURCE_CONTENT "#include \"embedded_shaders.h\"\n\n")
set(SOURCE_CONTENT "${SOURCE_CONTENT}namespace Liara::EmbeddedShaders {\n\n")

set(PROCESSED_COUNT 0)

foreach(SHADER_FILE ${SHADER_LIST})
    if(EXISTS "${SHADER_FILE}")
        get_filename_component(SHADER_NAME "${SHADER_FILE}" NAME)
        string(REPLACE "." "_" SHADER_VAR "${SHADER_NAME}")

        file(READ "${SHADER_FILE}" SHADER_CONTENT HEX)

        if(SHADER_CONTENT)
            string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," SHADER_ARRAY "${SHADER_CONTENT}")
            string(REGEX REPLACE ",$" "" SHADER_ARRAY "${SHADER_ARRAY}")

            set(HEADER_CONTENT "${HEADER_CONTENT}    extern const uint8_t ${SHADER_VAR}_data[];\n")
            set(HEADER_CONTENT "${HEADER_CONTENT}    extern const std::span<const uint8_t> ${SHADER_VAR};\n")
            set(HEADER_CONTENT "${HEADER_CONTENT}    extern const size_t ${SHADER_VAR}_size;\n\n")

            set(SOURCE_CONTENT "${SOURCE_CONTENT}    const uint8_t ${SHADER_VAR}_data[] = {${SHADER_ARRAY}};\n")
            set(SOURCE_CONTENT "${SOURCE_CONTENT}    const std::span<const uint8_t> ${SHADER_VAR}{${SHADER_VAR}_data, sizeof(${SHADER_VAR}_data)};\n")
            set(SOURCE_CONTENT "${SOURCE_CONTENT}    const size_t ${SHADER_VAR}_size = sizeof(${SHADER_VAR}_data);\n\n")

            math(EXPR PROCESSED_COUNT "${PROCESSED_COUNT} + 1")
            message(STATUS "Embedded shader: ${SHADER_NAME} (${SHADER_VAR})")
        else()
            message(WARNING "Empty shader file: ${SHADER_FILE}")
        endif()
    else()
        message(WARNING "Shader file not found: ${SHADER_FILE}")
    endif()
endforeach()

set(HEADER_CONTENT "${HEADER_CONTENT}}\n")
set(SOURCE_CONTENT "${SOURCE_CONTENT}}\n")

file(WRITE "${EMBED_HEADER}" "${HEADER_CONTENT}")
file(WRITE "${EMBED_SOURCE}" "${SOURCE_CONTENT}")

message(STATUS "Successfully embedded ${PROCESSED_COUNT} shaders")