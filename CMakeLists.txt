cmake_minimum_required(VERSION 3.10)

project(regen)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SRCS "src/main.c" "src/regen.c")

add_executable(regen ${SRCS})
target_include_directories(regen PRIVATE "include/")
target_compile_options(regen PRIVATE -Wall -Wextra -Wpedantic)
