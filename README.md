# Remora-RP2040-W5500

install Pico-SDK

https://github.com/raspberrypi/pico-sdk

clone this repo
`https://github.com/scottalford75/Remora-RP2040-W5500`

ensure your sdk pathe is available

`export PICO_SDK_PATH=<path to your sdk install>`

Pin wiring can be found here: port/ioLibrary_Driver/inc/w5x00_spi.h

`mkdir build`
`cd build`

pass your board to configure it properly
`cmake -DPICO_BOARD=pico_w ..`

finally run the build to generate the binary
`make`



