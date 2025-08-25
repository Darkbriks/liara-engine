include(CMakeDependentOption)

function(liara_detect_modules_support)
    set(LIARA_MODULES_SUPPORTED OFF)

    ####################
    # Ninja version check
    ####################
    if(CMAKE_GENERATOR STREQUAL "Ninja")
        find_program(NINJA_EXECUTABLE ninja)
        if(NINJA_EXECUTABLE)
            execute_process(
                    COMMAND ${NINJA_EXECUTABLE} --version
                    OUTPUT_VARIABLE NINJA_VERSION
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET
            )

            if(NINJA_VERSION VERSION_LESS "1.11.0")
                message(WARNING "Ninja ${NINJA_VERSION} detected - modules require 1.11+")
                message(WARNING "Disabling modules support")
                set_property(GLOBAL PROPERTY LIARA_MODULES_SUPPORTED OFF)
                set_property(GLOBAL PROPERTY LIARA_MODULES_ENABLED OFF)
                return()
            endif()
        endif()
    endif()

    ####################
    # Detect compiler support
    ####################
    if(MSVC)
        if(MSVC_VERSION GREATER_EQUAL 1936) # VS 2022 17.6+
            set(LIARA_MODULES_SUPPORTED ON)
            message(STATUS "MSVC ${MSVC_VERSION} - stable C++20 modules support detected")
        elseif(MSVC_VERSION GREATER_EQUAL 1932) # VS 2022 17.2+
            set(LIARA_MODULES_SUPPORTED ON)
            message(STATUS "MSVC ${MSVC_VERSION} - good C++20 modules support detected")
            message(WARNING "Some edge cases may occur. Consider upgrading to VS 2022 17.6+ for best stability")
        elseif(MSVC_VERSION GREATER_EQUAL 1929) # VS 2019 16.10+
            set(LIARA_MODULES_SUPPORTED ON)
            message(WARNING "MSVC ${MSVC_VERSION} - experimental C++20 modules support")
            message(WARNING "Modules may be unstable. Consider upgrading to VS 2022 17.2+")
        else()
            message(STATUS "MSVC ${MSVC_VERSION} detected - modules support requires at least MSVC 19.29 (VS 2019 16.10)")
            message(STATUS "Falling back to header-only mode")
        endif()
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "20.0")
        # TODO: Downgrade progressively to test support
        set(LIARA_MODULES_SUPPORTED ON)
        message(STATUS "Clang modules support detected")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "14.0")
        # TODO: Try enabling and test stability
        # set(LIARA_MODULES_SUPPORTED ON)
        message(STATUS "GCC modules support detected")
        message(WARNING "GCC modules support is temporarily disabled due to stability issues")
        message(STATUS "Use -DLIARA_FORCE_ENABLE_MODULES=ON to override if needed")
    else()
        message(STATUS "C++20 modules not supported by the current compiler")
    endif()

    set_property(GLOBAL PROPERTY LIARA_MODULES_SUPPORTED ${LIARA_MODULES_SUPPORTED})

    ####################
    # User option
    ####################
    cmake_dependent_option(LIARA_ENABLE_MODULES
            "Enable C++20 modules support"
            ${LIARA_MODULES_SUPPORTED}
            "LIARA_MODULES_SUPPORTED"
            OFF
    )

    option(LIARA_FORCE_NO_MODULES "Force disable modules" OFF)
    option(LIARA_FORCE_ENABLE_MODULES "Force enable modules" OFF)

    if(LIARA_FORCE_ENABLE_MODULES)
        set(LIARA_ENABLE_MODULES ON)
        message(STATUS "Modules force-enabled by user")
    elseif(LIARA_FORCE_NO_MODULES)
        set(LIARA_ENABLE_MODULES OFF)
        message(STATUS "Modules force-disabled by user")
    endif()

    #####################
    # Final output
    #####################
    if(LIARA_ENABLE_MODULES)
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/modules")
        message(STATUS "C++20 modules ENABLED")
    else()
        message(STATUS "Using traditional headers")
    endif()

    set_property(GLOBAL PROPERTY LIARA_MODULES_ENABLED ${LIARA_ENABLE_MODULES})
endfunction()