cmake_minimum_required(VERSION 3.29)
project(LiaraEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Vulkan
find_package(Vulkan REQUIRED)

# Include GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw-3.4)

# Include GLM
add_subdirectory(lib/glm-1.0.1)

# Include Vulkan, GLFW and GLM
include_directories(${Vulkan_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})

add_executable(LiaraEngine main.cpp
        Liara_Window.h
        Liara_Window.cpp
        FirstApp.h
        FirstApp.cpp
        Liara_Pipeline.cpp
        Liara_Pipeline.h
        Liara_Device.cpp
        Liara_Device.h
)

# Link Vulkan, GLFW and GLM
target_link_libraries(LiaraEngine ${Vulkan_LIBRARIES} glfw glm)

# Compile shaders
if(WIN32)
    add_custom_command(TARGET LiaraEngine POST_BUILD
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/compile.bat
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
elseif(UNIX)
    add_custom_command(TARGET LiaraEngine POST_BUILD
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/compile.sh
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# Copy shaders to build directory
file(GLOB SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.spv")
file(COPY ${SHADERS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders)