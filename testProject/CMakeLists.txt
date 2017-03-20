cmake_minimum_required(VERSION 3.6)
project(testProject)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

set(SOURCE_FILES main.c)
add_executable(testProject ${SOURCE_FILES})