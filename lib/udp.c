#include "udp.h"
#include "socket.h"
#include "util.h"
#include <stdio.h>

uint8_t udp_open(uint16_t port)
{
    uint8_t sockid = socket_alloc(SOCKET_TYPE_UDP);
    if (sockid == SOCKET_INVALID)
        return sockid;

    socket_set_port(sockid, port);
    socket_cmd(sockid, SOCKET_CMD_OPEN);

    return sockid;
}

uint8_t udp_ready(uint8_t sockid)
{
    return socket_status(sockid) == SOCKET_STATUS_UDP;
}

void udp_close(uint8_t sockid)
{
    socket_free(sockid);
}

void udp_send(uint8_t sockid, const uint8_t *destip, uint16_t destport, uint16_t length, const uint8_t *data)
{
    socket_set_dest_ip(sockid, destip);
    socket_set_dest_port(sockid, destport);

    socket_send(sockid, length, data);
}

int16_t udp_available(uint8_t sockid)
{
    uint16_t ret = socket_available(sockid);
    if (ret >= UDP_HEADER_SIZE) // If a packet is available, read its size.
    {
        uint8_t header[UDP_HEADER_SIZE];
        socket_recv(sockid, UDP_HEADER_SIZE, header, 1); // Leave the header in the RX buffer, hence the 1.
        ret = header[UDP_HEADER_LENGTH_MSB] << 8 | header[UDP_HEADER_LENGTH_LSB];
    }
    else
        ret = -1;
    return ret;
}

uint16_t udp_recv(uint8_t sockid, uint16_t bufsize, uint8_t *data)
{
    (void)data;
    uint8_t header[UDP_HEADER_SIZE];
    uint16_t packet_length = 0;
    uint16_t get = 0;
    uint16_t count = 0;
    uint16_t left = 0;

    if (socket_available(sockid) < 8)
        return 0;

    socket_recv(sockid, UDP_HEADER_SIZE, header, 0);
    packet_length = header[UDP_HEADER_LENGTH_MSB] << 8 | header[UDP_HEADER_LENGTH_LSB];
    get = MIN(packet_length, bufsize);
    count = socket_recv(sockid, get, data, 0);
    left = packet_length - count;
    if (left) // Is there some data left to discard ?
    {
        socket_recv(sockid, left, NULL, 0); // Don't save discarded data, passing NULL
    }
    
    return count;
}
