cmake_minimum_required(VERSION 4.0)
project(bar)

set(CMAKE_CXX_STANDARD 23)

find_package(argh REQUIRED)

add_library(bar INTERFACE)
target_include_directories(bar INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(cli src/main.cxx)
target_link_libraries(cli argh)
