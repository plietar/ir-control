#include "tcp.h"
#include "socket.h"
#include "util.h"
#include "nettypes.h"

net_socket_t tcp_open(uint16_t port)
{
    net_socket_t sockid = socket_alloc(SOCKET_TYPE_TCP);
    if (sockid == SOCKET_INVALID)
        return sockid;

    socket_set_port(sockid, port);
    socket_cmd(sockid, SOCKET_CMD_OPEN);

    return sockid;
}

void tcp_listen(net_socket_t sockid)
{
    socket_cmd(sockid, SOCKET_CMD_LISTEN);
}

net_socket_t tcp_accept(net_socket_t *sockid)
{
    uint8_t status = socket_status(*sockid);
    if (status == SOCKET_STATUS_ESTABLISHED)
    {
        net_socket_t client = *sockid;

        *sockid = tcp_open(socket_port(*sockid));
        if (*sockid != SOCKET_INVALID)
            tcp_listen(*sockid);
        return client;
    }

    return SOCKET_INVALID;
}

uint8_t tcp_connected(net_socket_t sockid)
{
    uint8_t status = socket_status(sockid);
    return status == SOCKET_STATUS_ESTABLISHED;
}

uint8_t tcp_listening(net_socket_t sockid)
{
    uint8_t status = socket_status(sockid);
    return status == SOCKET_STATUS_LISTEN;
}

void tcp_disconnect(net_socket_t sockid)
{
    socket_disconnect(sockid);
}

void tcp_close(net_socket_t sockid)
{
    socket_close(sockid);
}

net_size_t tcp_rx_available(net_socket_t sockid)
{
    return socket_rx_rsr(sockid);
}

net_size_t tcp_rx_read(net_socket_t sockid, net_offset_t offset, net_size_t count, uint8_t *data)
{
    return socket_rx_read(sockid, offset, count, data);
}

void tcp_rx_flush(net_socket_t sockid, net_size_t count)
{
    socket_rx_flush(sockid, count);
}

uint8_t tcp_tx_prepare(net_socket_t sockid)
{
    return socket_tx_prepare(sockid);
}

net_offset_t tcp_tx_write(net_socket_t sockid, net_offset_t offset, net_size_t count, const uint8_t *data)
{
    return socket_tx_write(sockid, offset, count, data);
}

void tcp_tx_flush(net_socket_t sockid)
{
    socket_tx_flush(sockid);
}

