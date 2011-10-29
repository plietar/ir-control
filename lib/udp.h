#ifndef _udp_h__
#define _udp_h__
#include <stdint.h>

/*
 * This is not the real UDP header. It is a simplified
 * version used by the W5100 to transmit basic information
 */
struct udp_w5100_header
{
    uint8_t destip[4];
    uint16_t destport;
    uint16_t length;
} __attribute((packed));

uint8_t udp_open(uint16_t port);
uint8_t udp_ready(uint8_t sockid);
void udp_close(uint8_t sockid);

/*
 * The following functions allow to send packet by transferring small parts to the w5100.
 * This way, you may use buffers smaller than packets
 *
 * First call udp_tx_prepare to setup the packet with destination ip and port
 * then call udp_tx_add as many times as you want to add data
 * finally call udp_tx_flush to send the packet.
 *
 */
uint8_t udp_tx_prepare(uint8_t sockid, const uint8_t *destip, uint16_t destport);
void udp_tx_write(uint8_t sockid, uint16_t offset, uint16_t length, const uint8_t *data);
void udp_tx_flush(uint8_t sockid);

// TODO: implement these functions
uint16_t udp_rx_available(uint8_t sockid);
uint16_t udp_rx_read(int8_t sockid, uint16_t read_offset, uint16_t count, uint8_t *data);
void udp_rx_flush(uint8_t sockid);

/*
 * These is the compatiblity functions
 */
// Sends a whole packet
void udp_send(uint8_t sockid, const uint8_t *destip, uint16_t destport, uint16_t length, const uint8_t *data);
int16_t udp_available(uint8_t sockid);
uint16_t udp_recv(uint8_t sockid, uint16_t max, uint8_t *data);
#endif
