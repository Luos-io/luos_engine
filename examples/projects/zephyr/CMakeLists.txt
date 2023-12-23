cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(luos_sample_on_zephyr)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE lib/led/led.c)
target_include_directories(app PRIVATE lib/led)
