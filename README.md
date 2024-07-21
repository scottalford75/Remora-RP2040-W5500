# Remora-RP2040-W5500

# install 

Download the latest firmware

https://github.com/scottalford75/Remora-RP2040-W5500/tree/main/firmware

Upload the remora-rp2040-2.0.0.uf2 file to your pico.

To configure the linuxcnc hal module and configs use the filse here:

Hal Files (run halcompile on these)
https://github.com/scottalford75/Remora-RT1052-cpp/tree/main/LinuxCNC/components/Remora-eth

and the configs:
https://github.com/scottalford75/Remora-RP2040-W5500/tree/main/LinuxCNC/Configs/remora-rp2040


# Troubleshooting:

Ensure you can ping the remora ethernet, the ip shoud be 10.10.10.10

This command is useful to see the state of the port
`sudo ethtool eth0`

If you are using dupont wires the spi bus speed may be too high, you can change it here: https://github.com/scottalford75/Remora-RP2040-W5500/blob/main/port/ioLibrary_Driver/src/w5x00_spi.c#L141

`spi_init(SPI_PORT, 50000 * 1000);`








