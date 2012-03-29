#include "udp.h"
#include "socket.h"
#include "util.h"
#include "nettypes.h"
#include <stdio.h>

net_socket_t udp_open(uint16_t port)
{
    net_socket_t sockid = socket_alloc(SOCKET_TYPE_UDP);
    if (sockid == SOCKET_INVALID)
        return sockid;

    socket_set_port(sockid, port);
    socket_cmd(sockid, SOCKET_CMD_OPEN);

    return sockid;
}

uint8_t udp_ready(net_socket_t sockid)
{
    return socket_status(sockid) == SOCKET_STATUS_UDP;
}

void udp_close(net_socket_t sockid)
{
    socket_close(sockid);
}


net_size_t udp_rx_available(net_socket_t sockid)
{
    net_size_t ret = socket_rx_rsr(sockid);
    if (ret >= (net_size_t)sizeof(struct udp_w5100_header)) // If a packet is available, read its data size.
    {
        struct udp_w5100_header header;
        socket_rx_read(sockid, 0, sizeof(struct udp_w5100_header), (uint8_t*)&header); // Don't call flush. We only want the data length 
        ret = util_ntohs(header.length);
    }
    else
        ret = -1;
    return ret;
}

void udp_rx_header(net_socket_t sockid, struct udp_w5100_header *out)
{
    net_size_t size = socket_rx_rsr(sockid);

    if (size >= (net_size_t)sizeof(struct udp_w5100_header)) // If a packet is available, read its data size.
    {
        socket_rx_read(sockid, 0, sizeof(struct udp_w5100_header), (uint8_t*)out); // Don't call flush. We only want the data length 
        out->destport = util_ntohs(out->destport);
    }
}

net_size_t udp_rx_read(int8_t sockid, net_offset_t read_offset, net_size_t bufsize, uint8_t *data)
{
    net_size_t packet_length = 0;
    net_size_t count = 0;


    packet_length = udp_rx_available(sockid);
    
    if (packet_length < 0)
        return packet_length;
    if (read_offset >= packet_length)
        return -1;

    count = MIN(packet_length, bufsize);
    count = socket_rx_read(sockid, sizeof(struct udp_w5100_header) + read_offset, count, data);
    return count;
}

void udp_rx_flush(net_socket_t sockid)
{
    net_size_t packet_length = 0;
    
    packet_length = udp_rx_available(sockid);

    if (packet_length < 0)
    {
        printf("No data to flush\n\r");
        return;
    }

    socket_rx_flush(sockid, sizeof(struct udp_w5100_header) + packet_length);
}


uint8_t udp_tx_prepare(net_socket_t sockid, const uint8_t *destip, uint16_t destport)
{
    uint8_t ret = 0;
    ret = socket_tx_prepare(sockid);
    if (ret)
    {
        socket_set_dest_ip(sockid, destip);
        socket_set_dest_port(sockid, destport);
    }
    return ret;
}

void udp_tx_write(net_socket_t sockid, net_offset_t offset, net_size_t length, const uint8_t *data)
{
    socket_tx_write(sockid, offset, length, data);
}

void udp_tx_flush(net_socket_t sockid)
{
    socket_tx_flush(sockid);
}

