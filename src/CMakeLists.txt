add_library(LiaraEngine STATIC)
add_library(Liara::Engine ALIAS LiaraEngine)

target_sources(LiaraEngine
        PRIVATE
        Core/Liara_App.cpp
        Core/Liara_Camera.cpp
        Core/Liara_SettingsManager.cpp
        Core/Liara_SignalHandler.cpp
        Core/Liara_Utils.h
        Core/Logging/Logger.cpp

        Graphics/Liara_Device.cpp
        Graphics/Liara_Pipeline.cpp
        Graphics/Liara_Model.cpp
        Graphics/Liara_Buffer.cpp
        Graphics/Liara_Texture.cpp
        Graphics/Liara_ShaderLoader.cpp
        Graphics/Liara_SwapChain.cpp
        Graphics/PrimitiveGenerator.cpp

        Graphics/Descriptors/Liara_Descriptor.cpp

        Graphics/Renderers/Liara_RendererManager.cpp
        Graphics/Renderers/Liara_ForwardRenderer.cpp

        Systems/SimpleRenderSystem.cpp
        Systems/PointLightSystem.cpp
        Systems/ImGuiSystem.cpp

        UI/ImGuiEngineStats.cpp
        UI/ImGuiLogConsole.cpp

        Plateform/Liara_Window.cpp

        Listener/KeybordMovementController.cpp

        PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
        Core/Liara_App.h
        Core/Liara_Camera.h
        Core/Liara_GameObject.h
        Core/FrameInfo.h

        Core/Logging/LogLevel.h
        Core/Logging/LogCategory.h
        Core/Logging/LogMessage.h
        Core/Logging/ThreadSafeQueue.h
        Core/Logging/Logger.h
        Core/Logging/LogMacros.h

        Graphics/Liara_Device.h
        Graphics/Liara_Model.h
)

liara_set_compiler_settings(LiaraEngine)

target_include_directories(LiaraEngine
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/external>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/external/imgui>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/external/imgui/backends>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(LiaraEngine
        PUBLIC
        Vulkan::Vulkan
        SDL2::SDL2
        fmt::fmt
        glm::glm
        PRIVATE
        tinyobjloader
        nlohmann_json::nlohmann_json
        stb::stb
)

target_sources(LiaraEngine PRIVATE
        ${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp
        ${CMAKE_SOURCE_DIR}/external/imgui/imgui_demo.cpp
        ${CMAKE_SOURCE_DIR}/external/imgui/imgui_draw.cpp
        ${CMAKE_SOURCE_DIR}/external/imgui/imgui_tables.cpp
        ${CMAKE_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
        ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_vulkan.cpp
        ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_sdl2.cpp
)

target_include_directories(LiaraEngine PRIVATE
        ${CMAKE_SOURCE_DIR}/external/imgui
        ${CMAKE_SOURCE_DIR}/external/imgui/backends
)

if(WIN32)
    target_compile_definitions(LiaraEngine PRIVATE LIARA_PLATFORM_WINDOWS)
elseif(UNIX)
    target_compile_definitions(LiaraEngine PRIVATE LIARA_PLATFORM_UNIX)
endif()

target_compile_definitions(LiaraEngine PRIVATE $<$<BOOL:${LIARA_EMBED_SHADERS}>:LIARA_EMBED_SHADERS>)

if(LIARA_EMBED_SHADERS)
    if(LIARA_EMBEDDED_SHADER_HEADER AND LIARA_EMBEDDED_SHADER_SOURCE)
        target_sources(LiaraEngine PRIVATE
                ${LIARA_EMBEDDED_SHADER_SOURCE}
        )

        get_filename_component(EMBED_INCLUDE_DIR "${LIARA_EMBEDDED_SHADER_HEADER}" DIRECTORY)
        target_include_directories(LiaraEngine PRIVATE
                ${EMBED_INCLUDE_DIR}
        )

        add_dependencies(LiaraEngine LiaraShaders)

        message(STATUS "LiaraEngine: Embedded shaders integration complete")
    else()
        message(WARNING "LiaraEngine: Embedded shader paths not found")
    endif()
endif()