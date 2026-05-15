# Remora-RP2040-W5500

## Install

Download the latest firmware

https://github.com/scottalford75/Remora-RP2040-W5500/tree/main/firmware

Upload the remora-rp2040-2.0.0.uf2 file to your pico.

To configure the linuxcnc hal module and configs use the files here:

Hal Files (run halcompile on these)
https://github.com/scottalford75/Remora-RT1052-cpp/tree/main/LinuxCNC/components/Remora-eth

and the configs:
https://github.com/scottalford75/Remora-RP2040-W5500/tree/main/LinuxCNC/Configs/remora-rp2040


## Troubleshooting:

Ensure you can ping the remora ethernet, the ip shoud be 10.10.10.10

This command is useful to see the state of the port
`sudo ethtool eth0`

If you are using dupont wires the spi bus speed may be too high, you can change it here: https://github.com/scottalford75/Remora-RP2040-W5500/blob/main/port/ioLibrary_Driver/src/w5x00_spi.c#L141

`spi_init(SPI_PORT, 50000 * 1000);`

If you are configured for the wrong board, ethernet may fail to initialize.
Different boards have the ethernet interface on different I/O lines.

Release builds currently do not work, make sure you are configured for a
debug build.

## Building

If you have [Nix](https://nixos.org/download/) installed (NixOS is not
necessary) and [flakes enabled](https://nixos.wiki/wiki/flakes), you can use
`nix develop` to enter a shell with all necessary build dependencies, pinned
build dependencies.

Otherwise, you will need cmake, GNU make, gcc-arm-embedded, and possibly
picotool, python3, and pkg-config.

```sh
$ rm -rf build
$ cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBOARD_NAME=<board-name>
$ cmake --build build
```

If you omit `-DBOARD_NAME`, it defaults to `W5500_EVB_PICO`.  Other
supported boards are listed at the top of `CMakeLists.txt`.  This should work
with all RP2040-based WIZnet boards (RP2350 ones do not build yet).  I've only
tested with:

* `W5500_EVB_PICO`
* `W55RP20_EVB_PICO`

The resulting firmware is in `build/remora.uf2`.
