cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(NEXUS C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_library(nexus_firmware STATIC
    _src/main.cpp
)

target_include_directories(nexus_firmware PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/_include
)

target_link_libraries(nexus_firmware PUBLIC
    pico_stdlib
    hardware_gpio
    hardware_i2c
    hardware_spi
)
