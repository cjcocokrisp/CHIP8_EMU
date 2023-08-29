cmake_minimum_required(VERSION 3.5.0)
project(CHIP8_EMU VERSION 0.1.0)

find_package(OpenGL REQUIRED)

set(EXECUTABLE_OUTPUT_PATH ../bin)
set(SOURCES main.c chip8.c glad.c)
add_executable(CHIP8_EMU ${SOURCES})

target_link_libraries(CHIP8_EMU glfw3 OpenGL::GL)
target_link_libraries(CHIP8_EMU winmm)