function(liara_set_compiler_settings target)
    target_compile_features(${target} PUBLIC cxx_std_20)

    if(MSVC)
        target_compile_options(${target} PRIVATE
                /W4 /WX                # Warnings as errors
                /permissive-           # Strict conformance
                /Zc:__cplusplus        # Correct __cplusplus macro
        )
    else()
        target_compile_options(${target} PRIVATE
                -Wall -Wextra -Wpedantic -Werror
                -Wno-unused-parameter  # Somtimes needed with Vulkan
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