find_program(LIARA_GLSLC_EXECUTABLE
        NAMES glslc
        HINTS
        ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
        ${VULKAN_SDK_PATH}/Bin
        ${VULKAN_SDK_PATH}/Bin32
        $ENV{VULKAN_SDK}/Bin
        $ENV{VULKAN_SDK}/Bin32
        ${VULKAN_SDK_PATH}/bin
        $ENV{VULKAN_SDK}/bin
        $ENV{VULKAN_SDK}/x86_64/bin
        /usr/bin
        /usr/local/bin
        /opt/vulkan-sdk/*/x86_64/bin
        PATHS
        ENV PATH
        DOC "Path to glslc executable"
)

if(NOT LIARA_GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc not found! Make sure Vulkan SDK is properly installed.")
endif()

message(STATUS "Found glslc: ${LIARA_GLSLC_EXECUTABLE}")

function(liara_compile_shader shader_source output_dir compiled_shader_var)
    get_filename_component(SHADER_NAME ${shader_source} NAME)
    set(COMPILED_SHADER "${output_dir}/${SHADER_NAME}.spv")

    set(COMPILE_ARGS
            -O
            --target-env=vulkan1.3
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(APPEND COMPILE_ARGS -g -O0)
    endif()

    add_custom_command(
            OUTPUT ${COMPILED_SHADER}
            COMMAND ${LIARA_GLSLC_EXECUTABLE}
            ${COMPILE_ARGS}
            ${shader_source}
            -o ${COMPILED_SHADER}
            DEPENDS ${shader_source}
            COMMENT "Compiling shader: ${SHADER_NAME}"
            VERBATIM
    )

    set(${compiled_shader_var} ${COMPILED_SHADER} PARENT_SCOPE)
endfunction()

function(liara_embed_shaders shader_files target_name)
    set(EMBED_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/embedded_shaders.cpp")
    set(EMBED_HEADER "${CMAKE_CURRENT_BINARY_DIR}/embedded_shaders.h")

    set(EMBED_CONTENT "#include \"embedded_shaders.h\"\n")
    set(HEADER_CONTENT "#pragma once\n#include <cstdint>\n#include <span>\n#include <string_view>\n\n")
    set(HEADER_CONTENT "${HEADER_CONTENT}namespace Liara::EmbeddedShaders {\n")

    foreach(SHADER_FILE ${shader_files})
        get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
        string(REGEX REPLACE "\\.(vert|frag|comp)$" "" SHADER_NAME_NO_EXT ${SHADER_NAME})
        string(MAKE_C_IDENTIFIER ${SHADER_NAME_NO_EXT} SHADER_VAR)

        file(READ ${SHADER_FILE} SHADER_CONTENT HEX)
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," SHADER_ARRAY ${SHADER_CONTENT})
        string(REGEX REPLACE ",$" "" SHADER_ARRAY ${SHADER_ARRAY})

        set(EMBED_CONTENT "${EMBED_CONTENT}static const uint8_t ${SHADER_VAR}_data[] = {${SHADER_ARRAY}}\;\n")
        set(EMBED_CONTENT "${EMBED_CONTENT}const std::span<const uint8_t> ${SHADER_VAR}{${SHADER_VAR}_data, sizeof(${SHADER_VAR}_data)}\;\n\n")

        set(HEADER_CONTENT "${HEADER_CONTENT}    extern const std::span<const uint8_t> ${SHADER_VAR}\;\n")
    endforeach()

    set(HEADER_CONTENT "${HEADER_CONTENT}}\n")

    file(WRITE ${EMBED_HEADER} ${HEADER_CONTENT})
    file(WRITE ${EMBED_SOURCE} ${EMBED_CONTENT})

    add_library(${target_name} STATIC ${EMBED_SOURCE})
    target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    liara_set_compiler_settings(${target_name})
endfunction()