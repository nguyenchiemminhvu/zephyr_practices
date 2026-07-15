cmake_minimum_required(VERSION 3.20)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test_kernel_iterable)

target_sources(app PRIVATE src/main.c)

zephyr_linker_sources(DATA_SECTIONS sections-ram.ld)
