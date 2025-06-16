include(FetchContent)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(FETCHCONTENT_QUIET OFF)

# Vulkan (system dependency)
find_package(Vulkan REQUIRED COMPONENTS glslc)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found! Please install Vulkan SDK 1.3+")
endif()

function(liara_fetch_dependency name git_repo git_tag)
    string(TOLOWER ${name} name_lower)
    FetchContent_Declare(
            ${name_lower}
            GIT_REPOSITORY ${git_repo}
            GIT_TAG ${git_tag}
            GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(${name_lower})
endfunction()

FetchContent_Declare(
        sdl2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG SDL2
        GIT_SHALLOW TRUE
)

set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(SDL2_DISABLE_SDL2MAIN OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(sdl2)

liara_fetch_dependency(fmt https://github.com/fmtlib/fmt.git 10.2.1)
liara_fetch_dependency(glm https://github.com/g-truc/glm.git 1.0.1)
liara_fetch_dependency(tinyobjloader https://github.com/tinyobjloader/tinyobjloader.git release)

if(NOT TARGET fmt::fmt)
    add_library(fmt::fmt ALIAS fmt)
endif()