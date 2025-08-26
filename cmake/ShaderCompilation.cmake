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

function(liara_create_embed_target shader_files target_name)
    set(EMBED_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/embedded_shaders.cpp")
    set(EMBED_HEADER "${CMAKE_CURRENT_BINARY_DIR}/embedded_shaders.h")

    get_filename_component(EMBED_SOURCE "${EMBED_SOURCE}" ABSOLUTE)
    get_filename_component(EMBED_HEADER "${EMBED_HEADER}" ABSOLUTE)

    add_custom_command(
            OUTPUT "${EMBED_HEADER}" "${EMBED_SOURCE}"
            COMMAND ${CMAKE_COMMAND}
            "-DSHADER_FILES=${shader_files}"
            "-DEMBED_HEADER=${EMBED_HEADER}"
            "-DEMBED_SOURCE=${EMBED_SOURCE}"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/EmbedShaders.cmake"
            DEPENDS ${shader_files}
            COMMENT "Embedding shaders into C++ code"
            VERBATIM
    )

    add_custom_target(${target_name}
            DEPENDS "${EMBED_HEADER}" "${EMBED_SOURCE}"
            COMMENT "Shader embedding complete"
    )

    set_target_properties(${target_name} PROPERTIES
            EMBED_HEADER_PATH "${EMBED_HEADER}"
            EMBED_SOURCE_PATH "${EMBED_SOURCE}"
    )

    message(STATUS "Embed target '${target_name}' configured:")
    message(STATUS "  Header: ${EMBED_HEADER}")
    message(STATUS "  Source: ${EMBED_SOURCE}")
endfunction()