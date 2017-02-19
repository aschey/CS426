cmake_minimum_required(VERSION 3.6)
project(assignment2_3_20)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

set(SOURCE_FILES main.c)

add_executable(assignment2_3_20 ${SOURCE_FILES})