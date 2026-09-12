cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test_linking_castle_2.0)

target_sources(app PRIVATE src/main.cpp)
target_include_directories(app PRIVATE include)

target_compile_options(app PRIVATE -Wall -Wextra -Wpedantic -DCASTLE_USING_STD_NEW=0)