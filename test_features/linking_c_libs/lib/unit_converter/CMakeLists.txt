cmake_minimum_required(VERSION 3.20)

add_library(unit_converter STATIC src/unit_converter.c)

target_include_directories(unit_converter PUBLIC include)