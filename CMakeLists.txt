cmake_minimum_required(VERSION 3.6)   # CMake version check
set(CMAKE_CXX_STANDARD 14)

project(renderer)
set(RENDERER_SOURCE_DIR )
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(RENDERER_MAJOR_VERSION 0)
set(RENDERER_MINOR_VERSION 1)
set(RENDERER_PATCH_VERSION 0)
set(RENDERER_VERSION "${RENDERER_VERSION_MAJOR}.${RENDERER_VERSION_MINOR}.${RENDERER_VERSION_RELEASE}")

configure_file(
        "${CMAKE_SOURCE_DIR}/src/RendererConfig.h.in"
        "${CMAKE_SOURCE_DIR}/src/RendererConfig.h"
)

include_directories("${PROJECT_SOURCE_DIR}/src")

file(GLOB SOURCES "src/*.cpp")

add_executable(renderer ${SOURCES})

