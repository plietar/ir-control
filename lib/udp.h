#ifndef _udp_h__
#define _udp_h__
#include <stdint.h>

#define UDP_HEADER_SIZE 8
#define UDP_HEADER_LENGTH_MSB 6
#define UDP_HEADER_LENGTH_LSB 7

uint8_t udp_open(uint16_t port);
uint8_t udp_ready(uint8_t sockid);
void udp_close(uint8_t sockid);

// Sends a whole packet
void udp_send(uint8_t sockid, const uint8_t *destip, uint16_t destport, uint16_t length, const uint8_t *data);

/*
 * The following functions allow to send packet by transferring small parts to the w5100.
 * This way, you may use buffers smaller than packets
 *
 * First call udp_tx_prepare to setup the packet with destination ip and port
 * then call udp_tx_add as many times as you want to add data
 * finally call udp_tx_flush to send the packet.
 *
 */

/*
 * udp_tx_prepare
 * Prepares the socket to send some data.
 * Fails if data is pending.
 * If the socket is already prepared, but no data has be added, the function re-prepares and
 * overwrites the previous preparation
 * 
 * Returns 0 on failure and 1 on success.
 */
uint8_t udp_tx_prepare(uint8_t sockid, const uint8_t *destip, uint16_t destport);
void udp_tx_add(uint8_t sockid, uint16_t length, uint16_t offset, const uint8_t *data);
void udp_tx_flush(uint8_t sockid);

int16_t udp_available(uint8_t sockid);
uint16_t udp_recv(uint8_t sockid, uint16_t max, uint8_t *data);
#endif
