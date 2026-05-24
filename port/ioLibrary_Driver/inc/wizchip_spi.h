/**
    Copyright (c) 2022 WIZnet Co.,Ltd

    SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef _WIZCHIP_SPI_H_
#define _WIZCHIP_SPI_H_

#include "board_list.h"

/**
    ----------------------------------------------------------------------------------------------------
    Macros
    ----------------------------------------------------------------------------------------------------
*/
/* SPI */
#if (DEVICE_BOARD_NAME == W55RP20_EVB_PICO)

#define USE_PIO
#define WIZNET_SPI_CLKDIV_MAJOR_DEFAULT   2
#define WIZNET_SPI_CLKDIV_MINOR_DEFAULT   0

#define PIN_CS 20
#define PIN_SCK 21
#define PIN_MOSI 23
#define PIN_MISO 22
#define PIN_INT 24
#define PIN_RST 25

#elif (DEVICE_BOARD_NAME == W6300_EVB_PICO || DEVICE_BOARD_NAME == W6300_EVB_PICO2)

#define USE_PIO
#define WIZNET_SPI_CLKDIV_MAJOR_DEFAULT   2
#define WIZNET_SPI_CLKDIV_MINOR_DEFAULT   0

#define PIN_INT                 15
#define PIN_CS                  16
#define PIO_SPI_SCK_PIN         17
#define PIO_SPI_DATA_IO0_PIN    18
#define PIO_SPI_DATA_IO1_PIN    19
#define PIO_SPI_DATA_IO2_PIN    20
#define PIO_SPI_DATA_IO3_PIN    21
#define PIN_RST                 22

#else
/* SPI */
#define SPI_PORT spi0
#define SPI_CLK  10

#define PIN_SCK 18
#define PIN_MOSI 19
#define PIN_MISO 16
#define PIN_CS 17
#define PIN_RST 20
#define PIN_INT 21

/* Use SPI DMA */
//#define USE_SPI_DMA // if you want to use SPI DMA, uncomment.
#endif


/**
    ----------------------------------------------------------------------------------------------------
    Functions
    ----------------------------------------------------------------------------------------------------
*/
/* wizchip */
/*! \brief Set CS pin
    \ingroup wizchip_spi

    Set chip select pin of spi0 to low(Active low).

    \param none
*/
static inline void wizchip_select(void);

/*! \brief Set CS pin
    \ingroup wizchip_spi

    Set chip select pin of spi0 to high(Inactive high).

    \param none
*/
static inline void wizchip_deselect(void);

/*! \brief Read from an SPI device, blocking
    \ingroup wizchip_spi

    Set spi_read_blocking function.
    Read byte from SPI to rx_data buffer.
    Blocks until all data is transferred. No timeout, as SPI hardware always transfers at a known data rate.

    \param none
*/
static uint8_t wizchip_read(void);

/*! \brief Write to an SPI device, blocking
    \ingroup wizchip_spi

    Set spi_write_blocking function.
    Write byte from tx_data buffer to SPI device.
    Blocks until all data is transferred. No timeout, as SPI hardware always transfers at a known data rate.

    \param tx_data Buffer of data to write
*/
static void wizchip_write(uint8_t tx_data);


#if (_WIZCHIP_ == W6100)
static void wizchip_read_buf(uint8_t* rx_data, datasize_t len);
static void wizchip_write_buf(uint8_t* tx_data, datasize_t len);
#endif

#ifdef USE_SPI_DMA
/*! \brief Configure all DMA parameters and optionally start transfer
    \ingroup wizchip_spi

    Configure all DMA parameters and read from DMA

    \param pBuf Buffer of data to read
    \param len element count (each element is of size transfer_data_size)
*/
static void wizchip_read_burst(uint8_t *pBuf, uint16_t len);

/*! \brief Configure all DMA parameters and optionally start transfer
    \ingroup wizchip_spi

    Configure all DMA parameters and write to DMA

    \param pBuf Buffer of data to write
    \param len element count (each element is of size transfer_data_size)
*/
static void wizchip_write_burst(uint8_t *pBuf, uint16_t len);
#endif

/*! \brief Enter a critical section
    \ingroup wizchip_spi

    Set ciritical section enter blocking function.
    If the spin lock associated with this critical section is in use, then this
    method will block until it is released.

    \param none
*/
static void wizchip_critical_section_lock(void);

/*! \brief Release a critical section
    \ingroup wizchip_spi

    Set ciritical section exit function.
    Release a critical section.

    \param none
*/
static void wizchip_critical_section_unlock(void);

/*! \brief Initialize SPI instances and Set DMA channel
    \ingroup wizchip_spi

    Set GPIO to spi0.
    Puts the SPI into a known state, and enable it.
    Set DMA channel completion channel.

    \param none
*/
void wizchip_spi_initialize(void);

/*! \brief Initialize a critical section structure
    \ingroup wizchip_spi

    The critical section is initialized ready for use.
    Registers callback function for critical section for WIZchip.

    \param none
*/
void wizchip_cris_initialize(void);

/*! \brief wizchip reset
    \ingroup wizchip_spi

    Set a reset pin and reset.

    \param none
*/
void wizchip_reset(void);

/*! \brief Initialize WIZchip
    \ingroup wizchip_spi

    Set callback function to read/write byte using SPI & QSPI.
    Set callback function for WIZchip select/deselect.
    Set memory size of wizchip and monitor PHY link status.

    \param none
*/
void wizchip_initialize(void);

/*! \brief Check chip version
    \ingroup wizchip_spi

    Get version information.

    \param none
*/
void wizchip_check(void);

/* Network */
/*! \brief Initialize network
    \ingroup wizchip_spi

    Set network information.

    \param net_info network information.
*/
void network_initialize(wiz_NetInfo net_info);

/*! \brief Print network information
    \ingroup wizchip_spi

    Print network information about MAC address, IP address, Subnet mask, Gateway, DHCP and DNS address.

    \param net_info network information.
*/
void print_network_information(wiz_NetInfo net_info);

/*! \brief Print IPv6 Address
    \ingroup wizchip_spi

    Print IPv6 Address.

    \param net_info network information.
*/
void print_ipv6_addr(uint8_t* name, uint8_t* ip6addr);

#endif /* _WIZCHIP_SPI_H_ */
