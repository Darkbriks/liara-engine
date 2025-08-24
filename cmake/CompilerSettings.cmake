function(liara_set_compiler_settings target)
    target_compile_features(${target} PUBLIC cxx_std_20)

    get_property(modules_enabled GLOBAL PROPERTY LIARA_MODULES_ENABLED)

    if(modules_enabled)
        if(MSVC)
            target_compile_options(${target} PRIVATE
                    /experimental:module
                    /module:stdIfcDir ${CMAKE_BINARY_DIR}/modules
            )
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-modules-ts>)
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_compile_options(${target} PRIVATE
                    -fmodules-ts
                    -Xclang -fmodules-embed-all-files
            )
        endif()

        target_compile_definitions(${target} PRIVATE LIARA_MODULES_ENABLED=1)
        message(STATUS "Applied modules flags to ${target}")
    else()
        message(STATUS "Using traditional headers for ${target}")
    endif()

    if(MSVC)
        if (MSVC_VERSION LESS 1925)
            message(FATAL_ERROR
                    "LiaraEngine requires MSVC 19.25 (Visual Studio 2019 16.5) or newer "
                    "for __VA_OPT__ support. Detected version: ${MSVC_VERSION}")
        endif()

        target_compile_options(${target} PRIVATE
                /W4 /WX
                /permissive-
                /Zc:__cplusplus
                /Zc:preprocessor
        )
    else()
        target_compile_options(${target} PRIVATE
                -Wall -Wextra -Wpedantic -Werror
                -Wno-unused-parameter
        )
    endif()

    # Optimisations release
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        if(MSVC)
            target_compile_options(${target} PRIVATE /O2 /Ob2)
        else()
            target_compile_options(${target} PRIVATE -O3 -DNDEBUG)
        endif()
    endif()

    # Debug info
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(LIARA_ENABLE_VALIDATION)
            target_compile_definitions(${target} PRIVATE LIARA_DEBUG=1)
        endif()
    endif()
endfunction()