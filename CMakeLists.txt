cmake_minimum_required(VERSION 3.20)
set(PROJECT_NAME complik)
project(${PROJECT_NAME} C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

set(
        SOURCES

        src/main.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
