/**
 * Copyright (c) 2022 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _W5x00_LWIP_H_
#define _W5x00_LWIP_H_

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include "lwip/netif.h"

/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
/* LWIP */
#define ETHERNET_MTU 1500

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/*! \brief send an ethernet packet
 *  \ingroup w5x00_lwip
 *
 *  It is used to send outgoing data to the socket.
 *
 *  \param sn socket number
 *  \param buf a pointer to the data to send
 *  \param len the length of data in packet
 *  \return he sent data size
 */
int32_t send_lwip(uint8_t sn, uint8_t *buf, uint16_t len);

/*! \brief read an ethernet packet
 *  \ingroup w5x00_lwip
 *
 *  It is used to read incoming data from the socket.
 *
 *  \param sn socket number
 *  \param buf a pointer buffer to read incoming data
 *  \param len the length of the data in the packet
 *  \return the real received data size
 */
int32_t recv_lwip(uint8_t sn, uint8_t *buf, uint16_t len);

/*! \brief callback function
 *  \ingroup w5x00_lwip
 *
 *  This function is called by ethernet_output() when it wants
 *  to send a packet on the interface. This function outputs
 *  the pbuf as-is on the link medium.
 *
 *  \param netif a pre-allocated netif structure
 *  \param p main packet buffer struct
 *  \return ERR_OK if data was sent.
 */
err_t netif_output(struct netif *netif, struct pbuf *p);

/*! \brief callback function
 *  \ingroup w5x00_lwip
 *
 *  Callback function for link.
 *
 *  \param netif a pre-allocated netif structure
 */
void netif_link_callback(struct netif *netif);

/*! \brief callback function
 *  \ingroup w5x00_lwip
 *
 *   Callback function for status.
 *
 *   \param netif a pre-allocated netif structure
 */
void netif_status_callback(struct netif *netif);

/*! \brief callback function
 *  \ingroup w5x00_lwip
 *
 *  Callback function that initializes the interface.
 *
 *  \param netif a pre-allocated netif structure
 *  \return ERR_OK if Network interface initialized
 */
err_t netif_initialize(struct netif *netif);

/*! \brief ethernet frame cyclic redundancy check
 *  \ingroup w5x00_lwip
 *
 *  Perform cyclic redundancy check on ethernet frame
 *
 *  \param data a pointer to the ethernet frame
 *  \param length the total length of ethernet frame
 *  \return an ethernet frame cyclic redundancy check result value
 */
static uint32_t ethernet_frame_crc(const uint8_t *data, int length);

#endif /* _W5x00_LWIP_H_ */
