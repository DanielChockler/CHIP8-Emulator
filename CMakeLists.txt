cmake_minimum_required(VERSION 3.15)
project(Chip8Emulator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")

find_package(glfw3 REQUIRED)
find_library(OPENGL_FRAMEWORK OpenGL REQUIRED)

add_executable(chip8_emu 
    src/main.cpp
    src/chip8/chip8.cpp
)

target_include_directories(chip8_emu PRIVATE src)

target_link_libraries(chip8_emu PRIVATE glfw ${OPENGL_FRAMEWORK})
