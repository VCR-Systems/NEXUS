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

file(GLOB NEXUS_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/_src/*.cpp
)

# 1. Change add_library to add_executable using your project name 'NEXUS'
add_executable(NEXUS 
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
)

# 2. This line tells CMake to generate the .uf2 file for your executable
pico_add_extra_outputs(NEXUS)