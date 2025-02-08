include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
message(STATUS "Local .env.cmake: ${LOCAL_ENV}")

cmake_minimum_required(VERSION 3.22)

set(NAME LiaraEngine)
set(LIARA_VERSION 0.16.0)

message(STATUS "using ${CMAKE_GENERATOR}")
if (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
    if (NOT MINGW_PATH)
        message(FATAL_ERROR "MINGW_PATH not set in .env.cmake")
    endif()
    set(USE_MINGW "True")
    set(CMAKE_C_COMPILER ${MINGW_PATH}/bin/gcc.exe)
    set(CMAKE_CXX_COMPILER  ${MINGW_PATH}/bin/g++.exe)
endif()

project(${NAME} VERSION ${LIARA_VERSION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# 2. SDL
if (DEFINED SDL_PATH)
    message(STATUS "Using SDL path specified in .env")
    add_subdirectory(${SDL_PATH})
else()
    message(STATUS "Using SDL from external")
    add_subdirectory(external/SDL2-2.30.8)
endif()

# 3. ImGui
if (DEFINED IMGUI_PATH)
    message(STATUS "Using ImGui path specified in .env")
    include_directories(${IMGUI_PATH})
    file(GLOB IMGUI_SOURCES "${IMGUI_PATH}/*.cpp" "${IMGUI_PATH}/backends/*.cpp")
else()
    message(STATUS "Using ImGui from external")
    include_directories(external/imgui)
    file(GLOB IMGUI_SOURCES "external/imgui/*.cpp" "external/imgui/backends/*.cpp")
endif()

# 4. TinyObjLoader
if (DEFINED TINYOBJ_PATH)
    message(STATUS "Using TinyObjLoader path specified in .env")
    include_directories(${TINYOBJ_PATH})
    file(GLOB_RECURSE TINYOBJ_SOURCES "${TINYOBJ_PATH}/*.h")
else()
    message(STATUS "Using TinyObjLoader from external")
    include_directories(external/tiny_obj_loader)
    file(GLOB_RECURSE TINYOBJ_SOURCES "external/tiny_obj_loader/*.h")
endif()

# 5. FMT
if (DEFINED FMT_PATH)
    message(STATUS "Using FMT path specified in .env")
    add_subdirectory(${FMT_PATH})
else()
    message(STATUS "Using FMT from external")
    add_subdirectory(external/fmt-11.1.3)
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h" "app/*.cpp" "app/*.h" "main.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

if (WIN32)
    message(STATUS "CREATING BUILD FOR WINDOWS")
    target_sources(${PROJECT_NAME} PRIVATE ${IMGUI_SOURCES} ${TINYOBJ_SOURCES})
    target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src ${Vulkan_INCLUDE_DIRS})
    target_link_directories(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} vulkan-1 SDL2::SDL2 fmt::fmt)

elseif (UNIX)
    message(STATUS "CREATING BUILD FOR UNIX")
    target_sources(${PROJECT_NAME} PRIVATE ${IMGUI_SOURCES} ${TINYOBJ_SOURCES})
    target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src)
    target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES} SDL2::SDL2 fmt::fmt)
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