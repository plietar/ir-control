#ifndef __tcp_h_
#define __tcp_h_

#include "nettypes.h"

net_socket_t tcp_open(uint16_t port);
void tcp_listen(net_socket_t sockid);
net_socket_t tcp_accept(net_socket_t *sockid);
uint8_t tcp_connected(net_socket_t sockid);
uint8_t tcp_listening(net_socket_t sockid);
void tcp_disconnect(net_socket_t sockid);
void tcp_close(net_socket_t sockid);

net_size_t tcp_rx_available(net_socket_t sockid);
net_size_t tcp_rx_read(net_socket_t sockid, net_offset_t offset, net_size_t count, uint8_t *data);
void tcp_rx_flush(net_socket_t sockid, net_size_t count);

uint8_t tcp_tx_prepare(net_socket_t sockid);
net_offset_t tcp_tx_write(net_socket_t sockid, net_offset_t offset, net_size_t count, const uint8_t *data);
void tcp_tx_flush(net_socket_t sockid);

#endif
