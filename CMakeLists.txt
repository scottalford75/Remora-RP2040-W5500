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

# Set board
set(BOARD_NAME W5500_EVB_PICO CACHE STRING "WIZnet board selection")
set_property(CACHE BOARD_NAME PROPERTY STRINGS
    WIZnet_Ethernet_HAT
    W5100S_EVB_PICO
    W5500_EVB_PICO
    W55RP20_EVB_PICO
    W6100_EVB_PICO
    W6300_EVB_PICO
    W5100S_EVB_PICO2
    W5500_EVB_PICO2
    W6100_EVB_PICO2
    W6300_EVB_PICO2
)

if(${BOARD_NAME} STREQUAL WIZnet_Ethernet_HAT)
    #set(PICO_PLATFORM rp2350) #If you are using WIZnet_ethernet_HAT with Pico2, uncomment it.
    set(WIZNET_CHIP W5100S)
    add_definitions(-D_WIZCHIP_=W5100S)
    add_definitions(-DDEVICE_BOARD_NAME=WIZnet_Ethernet_HAT)

elseif(${BOARD_NAME} STREQUAL W5100S_EVB_PICO)
    set(WIZNET_CHIP W5100S)
    add_definitions(-D_WIZCHIP_=W5100S)
    add_definitions(-DDEVICE_BOARD_NAME=W5100S_EVB_PICO)

elseif(${BOARD_NAME} STREQUAL W5500_EVB_PICO)
    set(WIZNET_CHIP W5500)
    add_definitions(-D_WIZCHIP_=W5500)
    add_definitions(-DDEVICE_BOARD_NAME=W5500_EVB_PICO)

elseif(${BOARD_NAME} STREQUAL W55RP20_EVB_PICO)
    set(WIZNET_CHIP W5500)
    add_definitions(-D_WIZCHIP_=W5500)
    add_definitions(-DDEVICE_BOARD_NAME=W55RP20_EVB_PICO)

elseif(${BOARD_NAME} STREQUAL W6100_EVB_PICO)
    set(WIZNET_CHIP W6100)
    add_definitions(-D_WIZCHIP_=W6100)
    add_definitions(-DDEVICE_BOARD_NAME=W6100_EVB_PICO)

elseif(${BOARD_NAME} STREQUAL W6300_EVB_PICO)
    set(WIZNET_CHIP W6300)
    add_definitions(-D_WIZCHIP_=W6300)
    add_definitions(-DDEVICE_BOARD_NAME=W6300_EVB_PICO)
# Set QSPI MODE for W6300
    set(WIZCHIP_QSPI_MODE QSPI_QUAD_MODE)
    # set(WIZCHIP_QSPI_MODE QSPI_DUAL_MODE)
    # set(WIZCHIP_QSPI_MODE QSPI_SINGLE_MODE)

    add_definitions(-D_WIZCHIP_QSPI_MODE_=${WIZCHIP_QSPI_MODE})

elseif(${BOARD_NAME} STREQUAL W5100S_EVB_PICO2)
    set(PICO_PLATFORM rp2350)
    set(WIZNET_CHIP W5100S)
    add_definitions(-D_WIZCHIP_=W5100S)
    add_definitions(-DDEVICE_BOARD_NAME=W5100S_EVB_PICO2)

elseif(${BOARD_NAME} STREQUAL W5500_EVB_PICO2)
    set(PICO_PLATFORM rp2350)
    set(WIZNET_CHIP W5500)
    add_definitions(-D_WIZCHIP_=W5500)
    add_definitions(-DDEVICE_BOARD_NAME=W5500_EVB_PICO2)

elseif(${BOARD_NAME} STREQUAL W6100_EVB_PICO2)
    set(PICO_PLATFORM rp2350)
    set(WIZNET_CHIP W6100)
    add_definitions(-D_WIZCHIP_=W6100)
    add_definitions(-DDEVICE_BOARD_NAME=W6100_EVB_PICO2)

elseif(${BOARD_NAME} STREQUAL W6300_EVB_PICO2)
    set(PICO_PLATFORM rp2350)
    set(WIZNET_CHIP W6300)
    add_definitions(-D_WIZCHIP_=W6300)
    add_definitions(-DDEVICE_BOARD_NAME=W6300_EVB_PICO2)
# Set QSPI MODE for W6300
    # Set QSPI MODE for W6300
    set(WIZCHIP_QSPI_MODE QSPI_QUAD_MODE)
    # set(WIZCHIP_QSPI_MODE QSPI_DUAL_MODE)
    # set(WIZCHIP_QSPI_MODE QSPI_SINGLE_MODE)

    add_definitions(-D_WIZCHIP_QSPI_MODE_=${WIZCHIP_QSPI_MODE})

else()
    message(FATAL_ERROR "BOARD_NAME is wrong = ${BOARD_NAME}")
endif()

message(STATUS "BOARD_NAME = ${BOARD_NAME}")
message(STATUS "WIZNET_CHIP = ${WIZNET_CHIP}")

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
        ${CMAKE_SOURCE_DIR}/drivers/pin/pin.cpp
        ${CMAKE_SOURCE_DIR}/modules/module.cpp
        ${CMAKE_SOURCE_DIR}/modules/comms/RemoraComms.cpp
        ${CMAKE_SOURCE_DIR}/modules/debug/debug.cpp
        ${CMAKE_SOURCE_DIR}/modules/blink/blink.cpp
        ${CMAKE_SOURCE_DIR}/modules/stepgen/stepgen.cpp
        ${CMAKE_SOURCE_DIR}/modules/digitalPin/digitalPin.cpp
        )

pico_generate_pio_header(remora
        ${CMAKE_SOURCE_DIR}/modules/stepgen/stepgen.pio
        )


target_include_directories(remora PUBLIC
        ${LWIP_PATH}/src/include
#        ${CMAKE_SOURCE_DIR}/interrupt
        )


target_link_libraries(remora 
        pico_stdlib
        pico_multicore
        hardware_spi
        hardware_pio
        hardware_dma
        hardware_pwm
        hardware_flash
        hardware_sync
        ETHERNET_FILES
        IOLIBRARY_FILES
        LWIP_FILES
        )

 # disable usb output, enable uart output        
pico_enable_stdio_usb(remora 0)
pico_enable_stdio_uart(remora 1)
        
pico_add_extra_outputs(remora)
