cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Path to Polyray's root directory.
set(POLYRAY_DIR "../..")

project(ExampleGame LANGUAGES CXX)

add_subdirectory(
    "${POLYRAY_DIR}"
    ${CMAKE_BINARY_DIR}/Polyray
)

add_executable(ExampleGame
    src/main.cpp
)

target_include_directories(ExampleGame PRIVATE
    src
)

target_link_libraries(ExampleGame PRIVATE
    Polyray
)

target_compile_options(ExampleGame PUBLIC
    -static-libgcc
    -static-libstdc++
    -static
)

# Automatically copies resources to the game's executable.
# Game resources are copied last, allowing them to override
# built-in Polyray resources.
add_custom_target(Resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/${POLYRAY_DIR}/res"
        "$<TARGET_FILE_DIR:ExampleGame>/res"

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/res"
        "$<TARGET_FILE_DIR:ExampleGame>/res"

    DEPENDS ExampleGame
)