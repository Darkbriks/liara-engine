cmake_minimum_required(VERSION 3.29)

set(ENGINE_NAME "LiaraEngine")
set(ENGINE_VERSION_MAJOR 0)
set(ENGINE_VERSION_MINOR 17)
set(ENGINE_VERSION_PATCH 0)

project(LiaraEngine
        VERSION ${ENGINE_VERSION_MAJOR}.${ENGINE_VERSION_MINOR}.${ENGINE_VERSION_PATCH}
        DESCRIPTION "Modern 3D graphics engine built with Vulkan"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CMakeDependentOption)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

function(liara_detect_modules_support)
    set(LIARA_MODULES_SUPPORTED OFF)

    if(MSVC)
        if (MSVC_VERSION GREATER_EQUAL 1928)  # VS 2019 16.8+
            set(LIARA_MODULES_SUPPORTED ON)
            message(STATUS "MSVC modules support detected")
        endif()
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "11.0")
            set(LIARA_MODULES_SUPPORTED ON)
            message(STATUS "GCC modules support detected")
        endif()
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "12.0")
            set(LIARA_MODULES_SUPPORTED ON)
            message(STATUS "Clang modules support detected (experimental)")
        endif()
    endif()

    set_property(GLOBAL PROPERTY LIARA_MODULES_SUPPORTED ${LIARA_MODULES_SUPPORTED})

    cmake_dependent_option(LIARA_ENABLE_MODULES
            "Enable C++20 modules support"
            ${LIARA_MODULES_SUPPORTED}
            "LIARA_MODULES_SUPPORTED"
            OFF
    )

    option(LIARA_FORCE_NO_MODULES "Force disable modules even if supported" OFF)

    if(LIARA_FORCE_NO_MODULES)
        set(LIARA_ENABLE_MODULES OFF)
        message(STATUS "Modules forcibly disabled by LIARA_FORCE_NO_MODULES")
    endif()

    if(LIARA_ENABLE_MODULES)
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/modules")

        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            set(MODULE_MAP_PATH "${CMAKE_BINARY_DIR}/module.map")
            if(NOT EXISTS "${MODULE_MAP_PATH}")
                file(WRITE "${MODULE_MAP_PATH}" "# Auto-generated module map for GCC\n")
                message(STATUS "Created module.map at ${MODULE_MAP_PATH}")
            endif()
        endif()

        message(STATUS "C++20 modules will be enabled")
    else()
        message(STATUS "C++20 modules will be disabled")
    endif()

    set_property(GLOBAL PROPERTY LIARA_MODULES_ENABLED ${LIARA_ENABLE_MODULES})

endfunction()

liara_detect_modules_support()

include(CompilerSettings)
include(Dependencies)

option(LIARA_BUILD_APPS "Build demo applications" ON)
option(LIARA_BUILD_TESTS "Build unit tests" OFF)
option(LIARA_EMBED_SHADERS "Embed shaders in executable" OFF)
option(LIARA_ENABLE_VALIDATION "Enable Vulkan validation layers in Debug" ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/Config.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/Config.h"
        @ONLY
)

add_subdirectory(external)
add_subdirectory(shaders)
add_subdirectory(src)

if(LIARA_BUILD_APPS)
    add_subdirectory(app)
endif()