#ifndef _socket_h__
#define _socket_h__

#include <stdint.h>
#include <stddef.h>
#include "nettypes.h"

#define SOCKET_SOCK0_RESERVED 0 // Should socket 0 be reserved for sock 0 only protocols ?
#define SOCKET_SOCK_COUNT 4

#define SOCKET_TMSR 0x55
#define SOCKET_RMSR 0x55

#define SOCKET_TYPE_CLOSED 0x0
#define SOCKET_TYPE_TCP 0x1
#define SOCKET_TYPE_UDP 0x2
#define SOCKET_TYPE_IPRAW 0x3
#define SOCKET_TYPE_MACRAW 0x4
#define SOCKET_TYPE_PPOE 0x5

#define SOCKET_TYPE_SOCK0 SOCKET_TYPE_MACRAW // Mac raw and above require socket 0

#define SOCKET_CMD_OPEN 0x01
#define SOCKET_CMD_LISTEN 0x02
#define SOCKET_CMD_CONNECT 0x04
#define SOCKET_CMD_DISCON 0x08
#define SOCKET_CMD_CLOSE 0x10
#define SOCKET_CMD_SEND 0x20
#define SOCKET_CMD_SEND_MAC 0x21
#define SOCKET_CMD_SEND_KEEP 0x22
#define SOCKET_CMD_RECV 0x40

#define SOCKET_STATUS_CLOSED 0x00
#define SOCKET_STATUS_INIT 0x13
#define SOCKET_STATUS_LISTEN 0x14
#define SOCKET_STATUS_ESTABLISHED 0x17
#define SOCKET_STATUS_CLOSE_WAIT 0x1C
#define SOCKET_STATUS_UDP 0x22
#define SOCKET_STATUS_IPRAW 0x32
#define SOCKET_STATUS_MACRAW 0x42
#define SOCKET_STATUS_PPOE 0x5F
#define SOCKET_STATUS_SYNSENT 0x15
#define SOCKET_STATUS_SYNRECV 0x16
#define SOCKET_STATUS_FIN_WAIT 0x18
#define SOCKET_STATUS_CLOSING 0x1A
#define SOCKET_STATUS_TIME_WAIT 0x1B
#define SOCKET_STATUS_LAST_ACK 0x1D
#define SOCKET_STATUS_ARP0 0x11
#define SOCKET_STATUS_ARP1 0x21
#define SOCKET_STATUS_ARP2 0x31

#define SOCKET_TX_BASE 0x4000U
#define SOCKET_TX_SIZE 0x0800U
#define SOCKET_TX_END 0x6000U

#define SOCKET_TX_BASE_S(sockid) (SOCKET_TX_BASE + SOCKET_TX_SIZE * sockid)
#define SOCKET_TX_MASK_S(sockid) (SOCKET_TX_SIZE - 1)
#define SOCKET_TX_END_S(sockid) (SOCKET_TX_BASE_S(sockid) + SOCKET_TX_MASK_S(sockid) + 1)

#define SOCKET_RX_BASE 0x6000U
#define SOCKET_RX_SIZE 0x0800U
#define SOCKET_RX_END 0x8000U

#define SOCKET_RX_BASE_S(sockid) (SOCKET_RX_BASE + SOCKET_RX_SIZE * sockid)
#define SOCKET_RX_MASK_S(sockid) (SOCKET_RX_SIZE - 1)
#define SOCKET_RX_END_S(sockid) (SOCKET_RX_BASE_S(sockid) + SOCKET_RX_MASK_S(sockid) + 1)

uint8_t socket_get_type(net_socket_t sockid);
void socket_set_type(net_socket_t sockid, uint8_t type);

net_socket_t socket_alloc(uint8_t type);
void socket_free(net_socket_t sockid);

void socket_cmd(net_socket_t sockid, uint8_t cmd);
uint8_t socket_status(net_socket_t sockid);

uint8_t socket_ir(net_socket_t sockid);
void socket_ir_clear(net_socket_t sockid, uint8_t ir);

void socket_set_port(net_socket_t sockid, uint16_t port);
void socket_set_dest_ip(net_socket_t sockid, const uint8_t *destip);
void socket_set_dest_port(net_socket_t sockid, uint16_t destport);

uint8_t socket_tx_prepare(net_socket_t sockid);
void socket_tx_write(net_socket_t sockid, uint16_t write_offset, uint16_t length, const uint8_t *data);
void socket_tx_flush(net_socket_t sockid);

uint16_t socket_tx_fsr(net_socket_t sockid);
uint16_t socket_tx_rd(net_socket_t sockid);
uint16_t socket_tx_wr(net_socket_t sockid);
void socket_tx_set_wr(net_socket_t sockid, uint16_t wr);

uint16_t socket_rx_read(net_socket_t sockid, uint16_t read_offset, uint16_t count, uint8_t *data);
uint16_t socket_rx_flush(net_socket_t sockid, uint16_t count);

uint16_t socket_rx_rsr(net_socket_t sockid);
uint16_t socket_rx_rd(net_socket_t sockid);
void socket_rx_set_rd(net_socket_t sockid, uint16_t rd);

#endif
