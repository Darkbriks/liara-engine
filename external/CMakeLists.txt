if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h")
    message(STATUS "ImGui not found in external/, downloading...")

    FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG v1.90.1
            GIT_SHALLOW TRUE
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    )
    FetchContent_MakeAvailable(imgui)
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/stb/stb_image.h")
    message(STATUS "STB not found, downloading...")

    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/stb")

    file(DOWNLOAD
            "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/stb/stb_image.h"
            SHOW_PROGRESS
    )
endif()

add_library(stb INTERFACE)
add_library(stb::stb ALIAS stb)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} )

FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

if(LIARA_BUILD_TESTS)
    FetchContent_Declare(
            catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.5.0
            GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(catch2)
endif()

message(STATUS "External dependencies configured:")
message(STATUS "  - ImGui: ${CMAKE_CURRENT_SOURCE_DIR}/imgui")
message(STATUS "  - STB: ${CMAKE_CURRENT_SOURCE_DIR}/stb")
message(STATUS "  - nlohmann_json: via FetchContent")
if(LIARA_BUILD_TESTS)
    message(STATUS "  - Catch2: via FetchContent")
endif()