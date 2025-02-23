include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
include(FetchContent)
message(STATUS "Local .env.cmake: ${LOCAL_ENV}")


cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


message(STATUS "using ${CMAKE_GENERATOR}")
if (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
    if (NOT MINGW_PATH)
        message(FATAL_ERROR "MINGW_PATH not set in .env.cmake")
    endif()
    set(USE_MINGW "True")
    set(CMAKE_C_COMPILER ${MINGW_PATH}/bin/gcc.exe)
    set(CMAKE_CXX_COMPILER  ${MINGW_PATH}/bin/g++.exe)
endif()


set(NAME LiaraEngine)
set(LIARA_VERSION 0.16.1)
project(${NAME} VERSION ${LIARA_VERSION})


FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG master
)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG master
)

FetchContent_Declare(
        tinyobjloader
        GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
        GIT_TAG release
)

FetchContent_Declare(
        sdl2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG SDL2
)

FetchContent_MakeAvailable(fmt glm tinyobjloader sdl2)


# 1. Vulkan
if (DEFINED VULKAN_SDK_PATH)
    set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
    set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib")
    set(Vulkan_FOUND "True")
else()
    find_package(Vulkan REQUIRED) # throws error if could not find Vulkan
    message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
endif()
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
else()
    message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARIES}")
endif()


# 2. ImGui
if (DEFINED IMGUI_PATH)
    message(STATUS "Using ImGui path specified in .env")
    file(GLOB IMGUI_SOURCES "${IMGUI_PATH}/*.cpp" "${IMGUI_PATH}/backends/*.cpp")
else()
    message(STATUS "Using ImGui from external")
    SET(IMGUI_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui")
    file(GLOB IMGUI_SOURCES "external/imgui/*.cpp" "external/imgui/backends/*.cpp")
endif()


file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h" "app/*.cpp" "app/*.h" "main.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_sources(${PROJECT_NAME} PRIVATE ${IMGUI_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src ${Vulkan_INCLUDE_DIRS} ${IMGUI_PATH} tinyobjloader external/stb)

if (WIN32)
    message(STATUS "CREATING BUILD FOR WINDOWS")
    target_link_directories(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} vulkan-1 sdl2 fmt tinyobjloader)

elseif (UNIX)
    message(STATUS "CREATING BUILD FOR UNIX")
    target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES} sdl2 fmt tinyobjloader)
endif()

############## Build SHADERS #######################

# Find all vertex and fragment sources within shaders directory
# taken from VBlancos vulkan tutorial
# https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters/CMakeLists.txt
if (NOT DEFINED GLSL_VALIDATOR)
    find_program(GLSL_VALIDATOR glslangValidator HINTS
            ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
            /usr/bin
            /usr/local/bin
            ${VULKAN_SDK_PATH}/Bin
            ${VULKAN_SDK_PATH}/Bin32
            $ENV{VULKAN_SDK}/Bin/
            $ENV{VULKAN_SDK}/Bin32/
    )
endif ()

# get all .vert and .frag files in shaders directory
file(GLOB_RECURSE GLSL_SOURCE_FILES "${PROJECT_SOURCE_DIR}/shaders/*.frag" "${PROJECT_SOURCE_DIR}/shaders/*.vert")

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
    add_custom_command( OUTPUT ${SPIRV} COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV} DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})