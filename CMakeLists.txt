cmake_minimum_required(VERSION 3.19)

project(katerm)

enable_testing()

if (MSVC)
    add_compile_options(/utf-8)
endif()


add_subdirectory(character_width)

# terminal-interface

add_library(terminal-interface INTERFACE)

target_sources(terminal-interface
    INTERFACE
        src/terminal.cpp
        src/terminal_screen.cpp
        src/position.cpp
        src/terminal_decoder.cpp
        src/terminal_instructee.cpp)

target_include_directories(terminal-interface
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(terminal-interface PROPERTIES
    CXX_EXTENSIONS OFF
)

target_compile_features(terminal-interface
    INTERFACE cxx_std_17)

target_link_libraries(terminal-interface
    INTERFACE character_width)


# terminal-static

add_library(terminal-static STATIC
    $<TARGET_PROPERTY:terminal-interface,INTERFACE_SOURCES>)

target_include_directories(terminal-static
    PUBLIC $<TARGET_PROPERTY:terminal-interface,INTERFACE_INCLUDE_DIRECTORIES>)

target_compile_features(terminal-static
    PUBLIC $<TARGET_PROPERTY:terminal-interface,INTERFACE_COMPILE_FEATURES>)

target_link_libraries(terminal-static
    PUBLIC $<TARGET_PROPERTY:terminal-interface,INTERFACE_LINK_LIBRARIES>)

add_subdirectory(terminfo)

add_subdirectory(extern/Catch2)

add_subdirectory(tests)

if (KATERM_FUZZING)
    add_subdirectory(fuzzing)
endif()
