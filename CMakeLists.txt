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
    devices/src/mpx5700gp.cpp
    devices/src/ADXL377.cpp
    devices/src/deviceConfig.cpp
)
# List your calculator source files
set(CALCULATOR_SOURCES
    calculator/src/calculator.cpp
    calculator/src/windowAverage.cpp
    calculator/src/IIRFilter.cpp
)
# List your writer source files
set(WRITER_SOURCES
    writer/src/writer.cpp
    writer/src/sdwriter.cpp
    writer/src/hw_config.c
)
# List your queue source files
set(QUEUE_SOURCES
    queue/src/circularQueue.cpp
)

# List your LED source files
set(LED_DRIVER_SOURCES
    led_driver/led_driver.cpp
    led_driver/ws2812.pio
)

# Add the FatFs_SPI library
add_subdirectory(no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)

# Add executable. Include the device source files
add_executable(main
    main.cpp
    ${DEVICE_SOURCES}
    ${CALCULATOR_SOURCES}
    ${WRITER_SOURCES}
    ${QUEUE_SOURCES}
    ${LED_DRIVER_SOURCES}
)

# Generate the C header from the .pio file for the 'main' target.
# This creates ws2812.pio.h which is included by ws2812_utils.c
# Ensure the path to your .pio file is correct.
pico_generate_pio_header(main ${CMAKE_CURRENT_LIST_DIR}/led_driver/ws2812.pio)

# Add the devices/include directory to the include path
target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    calculator/include
    devices/include
    writer/include
    queue/include
    common
    led_driver
)

pico_set_program_name(main "RocketClub2425C++Code")
pico_set_program_version(main "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(main 0)
pico_enable_stdio_usb(main 1)

# Add the standard library to the build
target_link_libraries(main
        pico_stdlib)

target_compile_options(main PRIVATE -O3)

# Add the standard include files to the build
target_include_directories(main PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(main
        hardware_i2c
        hardware_spi
        hardware_adc
        hardware_dma
        hardware_pio
        hardware_interp
        hardware_timer
        hardware_clocks
        hardware_sync
        hardware_flash
        FatFs_SPI
        pico_multicore
        )

pico_add_extra_outputs(main)