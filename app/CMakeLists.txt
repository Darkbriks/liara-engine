add_executable(Demo)
add_executable(Liara::Demo ALIAS Demo)

target_sources(Demo
        PRIVATE
        Demo/DemoApp.cpp
        Demo/DemoApp.h
        ${CMAKE_SOURCE_DIR}/main.cpp
)

liara_set_compiler_settings(Demo)

target_include_directories(Demo
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/external
)

target_link_libraries(Demo
        PRIVATE
        Liara::Engine
        SDL2::SDL2main
)

if(WIN32)
    set_target_properties(Demo PROPERTIES
            WIN32_EXECUTABLE $<CONFIG:Release>
    )

    add_custom_command(TARGET Demo POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL2::SDL2> $<TARGET_FILE_DIR:Demo>
            COMMENT "Copying SDL2 DLL"
    )
endif()

set(ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")
set(ASSETS_DEST_DIR "$<TARGET_FILE_DIR:Demo>/assets")

add_custom_command(TARGET Demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ASSETS_SOURCE_DIR}" "${ASSETS_DEST_DIR}"
        COMMENT "Copying assets to build directory"
)

add_dependencies(Demo LiaraShaders)

if(NOT LIARA_EMBED_SHADERS)
    add_custom_command(TARGET Demo POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_BINARY_DIR}/shaders" "$<TARGET_FILE_DIR:Demo>/shaders"
            COMMENT "Copying compiled shaders"
    )
endif()