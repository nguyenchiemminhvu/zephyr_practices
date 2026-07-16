cmake_minimum_required(VERSION 3.20)

add_library(simple_math STATIC src/simple_math.c)

target_include_directories(simple_math PUBLIC include)