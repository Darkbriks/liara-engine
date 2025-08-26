include(ShaderCompilation)

add_custom_target(LiaraShaders ALL
        COMMENT "Compiling all shaders"
)

file(GLOB_RECURSE VERTEX_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.vert")
file(GLOB_RECURSE FRAGMENT_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.frag")
file(GLOB_RECURSE COMPUTE_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.comp")

set(ALL_SHADERS ${VERTEX_SHADERS} ${FRAGMENT_SHADERS} ${COMPUTE_SHADERS})

set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")

set(COMPILED_SHADERS)
foreach(SHADER ${ALL_SHADERS})
    liara_compile_shader(${SHADER} ${SHADER_OUTPUT_DIR} COMPILED_SHADER)
    list(APPEND COMPILED_SHADERS ${COMPILED_SHADER})
endforeach()

add_custom_target(LiaraShadersFiles
        DEPENDS ${COMPILED_SHADERS}
        COMMENT "Shader compilation complete"
)

add_dependencies(LiaraShaders LiaraShadersFiles)

if(LIARA_EMBED_SHADERS)
    liara_create_embed_target("${COMPILED_SHADERS}" LiaraShadersEmbedding)

    add_dependencies(LiaraShaders LiaraShadersEmbedding)
    add_dependencies(LiaraShadersEmbedding LiaraShadersFiles)

    get_target_property(EMBED_HEADER_PATH LiaraShadersEmbedding EMBED_HEADER_PATH)
    get_target_property(EMBED_SOURCE_PATH LiaraShadersEmbedding EMBED_SOURCE_PATH)

    set(LIARA_EMBEDDED_SHADER_HEADER "${EMBED_HEADER_PATH}" CACHE INTERNAL "Path to embedded shader header")
    set(LIARA_EMBEDDED_SHADER_SOURCE "${EMBED_SOURCE_PATH}" CACHE INTERNAL "Path to embedded shader source")

else()
    message(STATUS "Shader embedding disabled")
endif()