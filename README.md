# Remora-RP2040-W5500

# install 

download and install Pico-SDK

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

to create a debug build run
`cmake -DCMAKE_BUILD_TYPE=Debug . -DPICO_BOARD=pico`

finally run the build to generate the binary
```
make clean
make
```

Upload the remora.uf2 file to your pico.

To configure the linuxcnc hal module and configs use the filse here:

Hal Files (run halcompile on these)
https://github.com/scottalford75/Remora-NVEM/tree/main/LinuxCNC/Components/Remora-nv

and the configs:
https://github.com/scottalford75/Remora-NVEM/tree/main/LinuxCNC/ConfigSamples/remora-nvem-basic

As of commit 9ff31af100d31f97e95ae29b67da57f6c258ed4a the matching commit in the Remora-NVEM repo was 4b518891c1253de9fbf4fbbc11ab83a45523405b

# Troubleshooting:

Ensure you can ping the remora ethernet, the ip shoud be 10.10.10.10

The W5500 doesn't have MDI-X, ensure you are using a crossover cable and not a straight through cable.

this command is useful to see the state of the port
`sudo ethtool eth0`

If you are using dupont wires the spi bus speed may be too high, you can change it here: https://github.com/scottalford75/Remora-RP2040-W5500/blob/main/port/ioLibrary_Driver/src/w5x00_spi.c#L141

`spi_init(SPI_PORT, 50000 * 1000);`








