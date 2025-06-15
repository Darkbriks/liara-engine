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
    liara_embed_shaders("${COMPILED_SHADERS}" LiaraShadersEmbedded)
    add_dependencies(LiaraShadersEmbedded LiaraShadersFiles)
endif()