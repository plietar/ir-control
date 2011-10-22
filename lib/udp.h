#ifndef _udp_h__
#define _udp_h__
#include <stdint.h>

#define UDP_HEADER_SIZE 8
#define UDP_HEADER_LENGTH_MSB 6
#define UDP_HEADER_LENGTH_LSB 7

uint8_t udp_open(uint16_t port);
uint8_t udp_ready(uint8_t sockid);
void udp_close(uint8_t sockid);
void udp_send(uint8_t sockid, const uint8_t *destip, uint16_t destport, uint16_t length, const uint8_t *data);
int16_t udp_available(uint8_t sockid);
uint16_t udp_recv(uint8_t sockid, uint16_t max, uint8_t *data);
#endif
