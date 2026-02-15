cmake_minimum_required(VERSION 3.5.0)
project(file_server VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(GCC_compile_flags "-Wall -Wextra -Wpedantic -Werror -g")
add_definitions(${GCC_compile_flags})
add_compile_options(-fsanitize=undefined,address,leak)
add_link_options(-fsanitize=undefined,address,leak)

file (GLOB SOURCES
    main.cpp
)
add_executable(file_server ${SOURCES})
target_link_libraries(file_server PRIVATE cpp-httplib)
