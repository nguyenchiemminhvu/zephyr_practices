cmake_minimum_required(VERSION 3.20)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test_linking_c_libs)

target_sources(app PRIVATE src/main.c)

add_subdirectory(lib/simple_math)
add_subdirectory(lib/unit_converter)

target_link_libraries(app PRIVATE simple_math unit_converter)
