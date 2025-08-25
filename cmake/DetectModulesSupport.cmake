include(CMakeDependentOption)

function(liara_detect_modules_support)
    set(LIARA_MODULES_SUPPORTED OFF)

    ####################
    # Temporary workaround for Ubuntu
    ####################
    if(UNIX AND NOT APPLE)
        if(EXISTS "/etc/lsb-release")
            file(READ "/etc/lsb-release" LSB_RELEASE_CONTENT)
            if(LSB_RELEASE_CONTENT MATCHES "Ubuntu")
                message(WARNING "Clang modules support on Ubuntu is currently broken due to missing libc++ module maps.")
                message(WARNING "Falling back to header-only mode.")
                message(STATUS "Use -DLIARA_FORCE_ENABLE_MODULES=ON to override if needed")
                set_property(GLOBAL PROPERTY LIARA_MODULES_SUPPORTED OFF)
                set_property(GLOBAL PROPERTY LIARA_MODULES_ENABLED OFF)
                return()
            endif()
        endif()
    endif()

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
        if(MSVC_VERSION GREATER_EQUAL 1936)
            set(LIARA_MODULES_SUPPORTED ON)
            message(STATUS "MSVC ${MSVC_VERSION} - full C++20 modules support detected")
        elseif(MSVC_VERSION GREATER_EQUAL 1928)
            set(LIARA_MODULES_SUPPORTED ON)
            message(STATUS "MSVC ${MSVC_VERSION} - experimental C++20 modules support detected")
            message(WARNING "MSVC modules support is experimental and may be unstable")
            message(WARNING "Consider using at least MSVC 19.36 (Visual Studio 2022 17.6) for better stability")
            message(STATUS "Use -DLIARA_FORCE_NO_MODULES=ON to disable modules")
        else()
            message(STATUS "MSVC ${MSVC_VERSION} detected - modules support requires at least MSVC 19.28 (Visual Studio 2019 16.8)")
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