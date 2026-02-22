cmake_minimum_required(VERSION 3.5.0)
project(file_server VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc-14)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++-14)

add_compile_options(-static)
add_link_options(-static)

file (GLOB SOURCES
    httplib.h
    main.cpp
)
add_executable(file_server ${SOURCES})
