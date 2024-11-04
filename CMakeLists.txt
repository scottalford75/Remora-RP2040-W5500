# CMake minimum required version
cmake_minimum_required(VERSION 3.12)

# Include cmake import file
include(pico_sdk_import.cmake)
include(pico_extras_import_optional.cmake)

project(Remora-RP2040-W5500)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(PICO_COPY_TO_RAM 1)

#set(PICO_DEOPTIMIZED_DEBUG 1)

# Initialize the SDK
pico_sdk_init()

## Wiznet stuff
# Set ethernet chip
set(WIZNET_CHIP W5500)

if(${WIZNET_CHIP} STREQUAL W5100S)
    add_definitions(-D_WIZCHIP_=W5100S)
elseif(${WIZNET_CHIP} STREQUAL W5500)
    add_definitions(-D_WIZCHIP_=W5500)
else()
    message(FATAL_ERROR "WIZNET_CHIP is wrong = ${WIZNET_CHIP}")
endif()

if(NOT DEFINED WIZNET_DIR)
    set(WIZNET_DIR ${CMAKE_SOURCE_DIR}/libraries/ioLibrary_Driver)
    message(STATUS "WIZNET_DIR = ${WIZNET_DIR}")
endif()

if(NOT DEFINED PORT_DIR)
    set(PORT_DIR ${CMAKE_SOURCE_DIR}/port)
    message(STATUS "PORT_DIR = ${PORT_DIR}")
endif()

## end Wiznet stuff


# LWIP
set(LWIP_PATH ${PICO_SDK_PATH}/lib/lwip)

target_include_directories(pico_lwip INTERFACE        
        ${PORT_DIR}/lwip        
        )


# Add libraries in subdirectories
add_subdirectory(${CMAKE_SOURCE_DIR}/libraries)
add_subdirectory(${PORT_DIR})


include_directories(
        #${CMAKE_SOURCE_DIR}/interrupt
        )


add_executable(remora 
        remora.cpp
        tftpserver.cpp
        #boardconfig.cpp
        ${CMAKE_SOURCE_DIR}/interrupt/interrupt.cpp
        ${CMAKE_SOURCE_DIR}/thread/timerInterrupt.cpp
        ${CMAKE_SOURCE_DIR}/thread/timer.cpp
        ${CMAKE_SOURCE_DIR}/thread/pruThread.cpp
        ${CMAKE_SOURCE_DIR}/drivers/pin/pin.cpp
        ${CMAKE_SOURCE_DIR}/modules/module.cpp
        ${CMAKE_SOURCE_DIR}/modules/moduleInterrupt.cpp
        ${CMAKE_SOURCE_DIR}/modules/comms/RemoraComms.cpp
        ${CMAKE_SOURCE_DIR}/modules/debug/debug.cpp
        ${CMAKE_SOURCE_DIR}/modules/blink/blink.cpp
        ${CMAKE_SOURCE_DIR}/modules/stepgen/stepgen.cpp
        ${CMAKE_SOURCE_DIR}/modules/digitalPin/digitalPin.cpp
        )


target_include_directories(remora PUBLIC
        ${LWIP_PATH}/src/include
#        ${CMAKE_SOURCE_DIR}/interrupt
        )


target_link_libraries(remora 
        pico_stdlib
        pico_multicore
        hardware_spi
        hardware_dma
        hardware_pwm
        hardware_flash
        hardware_sync
        ETHERNET_FILES
        IOLIBRARY_FILES
        LWIP_FILES
        )

 # disable usb output, enable uart output        
pico_enable_stdio_usb(remora 1)
pico_enable_stdio_uart(remora 0)
        
pico_add_extra_outputs(remora)
