cmake_minimum_required(VERSION 3.20)

project(Polyray LANGUAGES C CXX)

add_library(Polyray STATIC
    src/glad/glad.c

    src/fastgltf/base64.cpp
    src/fastgltf/fastgltf.cpp
    src/fastgltf/io.cpp
    
    src/simdjson/simdjson.cpp

    src/stb_image.cpp

    src/ecs/ComponentRegistry.cpp

    src/scene/2d/Scene2D.cpp
    src/scene/3d/Scene3D.cpp
)

target_compile_features(Polyray PUBLIC cxx_std_20)

target_include_directories(Polyray PUBLIC
    src
)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

FetchContent_MakeAvailable(glfw)

target_link_libraries(Polyray PUBLIC
    glfw
)

target_compile_options(Polyray PRIVATE
    -Wall
    -O3
)

target_compile_options(Polyray PUBLIC
    -mavx2
    -mfma
)