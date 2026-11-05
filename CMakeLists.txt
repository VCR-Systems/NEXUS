# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(NEXUS C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

set(NO_OS_FATFS_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/no-OS-FatFS-SD-SPI-RPi-Pico)
add_subdirectory(${NO_OS_FATFS_PATH} no-OS-FatFS)

file(GLOB NEXUS_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/_src/*.cpp
)

add_executable(NEXUS 
    main.cpp
    hw_config.c
    ${NEXUS_SOURCES}
)

target_include_directories(NEXUS PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/_include
)

target_link_libraries(NEXUS PRIVATE
    pico_stdlib
    hardware_gpio
    hardware_i2c
    hardware_spi
    ff_stdio
)

# Enable USB output, disable UART output for printf
pico_enable_stdio_usb(NEXUS 1)
pico_enable_stdio_uart(NEXUS 0)

# 2. This line tells CMake to generate the .uf2 file for your executable
pico_add_extra_outputs(NEXUS)