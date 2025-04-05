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
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(RocketClub2425C++Code C CXX ASM)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# List your device source files
set(DEVICE_SOURCES
    devices/src/bmp280.cpp
    devices/src/gtu7.cpp
    devices/src/mpu6050.cpp
    devices/src/mpx5700gp.cpp
)
# List your calculator source files
set(CALCULATOR_SOURCES
    calculator/src/calculator.cpp
    calculator/src/windowAverage.cpp
)
# List your dma source files
set(DMAMANAGER_SOURCES
    dma/src/dmaManager.cpp
)
# List your sampler source files
set(SAMPLER_SOURCES
    sampler/src/dataSampler.cpp
)
# List your writer source files
set(WRITER_SOURCES
    writer/src/writer.cpp
    writer/src/hw_config.c
)

# Add the FatFs_SPI library
add_subdirectory(no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)

# Add executable. Include the device source files
add_executable(main
    main.cpp
    ${DEVICE_SOURCES}
    ${CALCULATOR_SOURCES}
    ${DMAMANAGER_SOURCES}
    ${SAMPLER_SOURCES}
    ${WRITER_SOURCES}
)

# Add the devices/include directory to the include path
target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    calculator/include
    devices/include
    dma/include
    sampler/include
    writer/include
    common
)

pico_set_program_name(main "RocketClub2425C++Code")
pico_set_program_version(main "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(main 0)
pico_enable_stdio_usb(main 1)

# Add the standard library to the build
target_link_libraries(main
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(main PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(main
        hardware_spi
        hardware_i2c
        hardware_dma
        hardware_pio
        hardware_interp
        hardware_timer
        hardware_clocks
        FatFs_SPI
        )

pico_add_extra_outputs(main)