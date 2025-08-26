function(liara_set_compiler_settings target)
    target_compile_features(${target} PUBLIC cxx_std_20)

    get_property(modules_enabled GLOBAL PROPERTY LIARA_MODULES_ENABLED)

    if(modules_enabled)
        if(MSVC)
            if(MSVC_VERSION LESS 1929) # Visual Studio 2019 16.10
                message(WARNING "MSVC ${MSVC_VERSION} has limited C++20 modules support. Recommended: VS 2022 17.0+")
            endif()

            file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/modules")

            target_compile_options(${target} PRIVATE
                    /std:c++20
                    /experimental:module
            )

            if(MSVC_VERSION GREATER_EQUAL 1930)
                target_compile_options(${target} PRIVATE
                        /interface
                        /ifcOutput ${CMAKE_BINARY_DIR}/modules/
                        /ifcSearchDir ${CMAKE_BINARY_DIR}/modules/
                )
            else()
                target_compile_options(${target} PRIVATE
                        /module:stdIfcDir ${CMAKE_BINARY_DIR}/modules
                        /module:output ${CMAKE_BINARY_DIR}/modules
                )
            endif()

        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_compile_options(${target} PRIVATE
                    -std=c++20
                    -fmodules
                    -fbuiltin-module-map
                    -fimplicit-module-maps
            )

            file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/clang-modules")

            target_compile_options(${target} PRIVATE
                    -fmodules-cache-path=${CMAKE_BINARY_DIR}/clang-modules
            )
        endif()

        target_compile_definitions(${target} PRIVATE
                LIARA_MODULES_ENABLED=1
                LIARA_UTILS_MODULE_AVAILABLE=1
        )
        message(STATUS "Applied modules flags to ${target}")
    else()
        message(STATUS "Using traditional headers for ${target}")
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${target} PRIVATE
                -Wall -Wextra -Wpedantic -Werror
                -Wno-unused-parameter
                -Wno-c++98-compat
                -Wno-c++98-compat-pedantic
                -Wno-include-angled-in-module-purview
                -Wno-ambiguous-macro
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE
                -Wall -Wextra -Wpedantic -Werror
                -Wno-unused-parameter
        )
    elseif(MSVC)
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
                /utf-8
                /MP
        )

        if(modules_enabled)
            target_compile_options(${target} PRIVATE
                    /wd5050  # Possible incompatible environment while importing module
                    /wd5105  # Macro expansion producing 'defined' has undefined behavior
            )
        endif()
    endif()

    # Optimisations release
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        if(MSVC)
            target_compile_options(${target} PRIVATE
                    /O2        # Maximize speed
                    /Ob2       # Inline expansion
                    /Oi        # Enable intrinsic functions
                    /Ot        # Favor fast code
                    /GL        # Whole program optimization
            )
            set_target_properties(${target} PROPERTIES
                    LINK_FLAGS_RELEASE "/LTCG /OPT:REF /OPT:ICF"
            )
        else()
            target_compile_options(${target} PRIVATE -O3 -DNDEBUG)
        endif()
    endif()

    # Debug info
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(MSVC)
            target_compile_options(${target} PRIVATE
                    /Od        # Disable optimizations
                    /Zi        # Generate complete debugging information
                    /RTC1      # Enable run-time error checks
            )
        endif()

        if(LIARA_ENABLE_VALIDATION)
            target_compile_definitions(${target} PRIVATE
                    LIARA_DEBUG=1
                    LIARA_ENABLE_ASSERTS=1
            )
        endif()
    endif()
endfunction()