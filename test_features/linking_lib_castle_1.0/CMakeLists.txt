cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test_linking_cpp_libs)

target_sources(app PRIVATE src/main.cpp)
target_include_directories(app PRIVATE include)