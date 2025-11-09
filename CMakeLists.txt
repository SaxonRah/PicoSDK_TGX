# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pimoroni_pico_plus2_rp2350 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(pgx C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(pgx)

# TGX library source files (C++ files that need compilation)
set(TGX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/tgx/Color.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tgx/Fonts.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tgx/Renderer3D.cpp

    ${CMAKE_CURRENT_LIST_DIR}/tgx/font_tgx_Arial.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tgx/font_tgx_Arial_Bold.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tgx/font_tgx_OpenSans.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tgx/font_tgx_OpenSans_Bold.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tgx/font_tgx_OpenSans_Italic.cpp
)

target_sources(pgx PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/lib/assert.c
    ${CMAKE_CURRENT_LIST_DIR}/ili9341/ili9341_tgx.cpp  # Changed to new TGX-compatible driver
    ${TGX_SOURCES}  # Add TGX source files

    # ${CMAKE_CURRENT_LIST_DIR}/pgx.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/pgx_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/pgx_stress.cpp
)

pico_set_program_name(pgx "pgx")
pico_set_program_version(pgx "0.1")

pico_enable_stdio_uart(pgx 1)
pico_enable_stdio_usb(pgx 0)

# Add the standard include files to the build
target_include_directories(pgx PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/..
    # ${CMAKE_CURRENT_LIST_DIR}/lib       # Add lib include directory
    # ${CMAKE_CURRENT_LIST_DIR}/ili9341   # Add ili9341 include directory
    ${CMAKE_CURRENT_LIST_DIR}/tgx       # Add TGX include directory
)

#target_compile_options(pgx PRIVATE
#    -O3              # Maximum optimization for graphics
#    -funroll-loops   # Loop optimization
#)

# TGX configuration defines (optional, adjust as needed)
target_compile_definitions(pgx PRIVATE
    TGX_RGB565_ORDER_BGR=1  # Match ILI9341 display byte order
)

# pull in common dependencies
target_link_libraries(pgx
    pico_stdlib
    pico_sync
    hardware_spi
    hardware_timer
    hardware_clocks
)

if (PICO_CYW43_SUPPORTED)
    target_link_libraries(pgx pico_cyw43_arch_none)
endif()

# create map/bin/hex file etc.
pico_add_extra_outputs(pgx)