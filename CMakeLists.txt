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

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(DetectModulesSupport)
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