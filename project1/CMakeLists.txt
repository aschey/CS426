cmake_minimum_required(VERSION 3.6)
project(project1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

set(SOURCE_FILES main.c)
add_executable(project1 ${SOURCE_FILES})