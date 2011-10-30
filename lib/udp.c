#include "udp.h"
#include "socket.h"
#include "util.h"
#include "nettypes.h"

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
    socket_free(sockid);
}


// TODO: implement these functions
// uint16_t udp_rx_available(net_socket_t sockid);
// uint16_t udp_rx_read(int8_t sockid, uint16_t read_offset, uint16_t count, uint8_t *data);
// void udp_rx_flush(net_socket_t sockid);

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

void udp_tx_write(net_socket_t sockid, uint16_t offset, uint16_t length, const uint8_t *data)
{
    socket_tx_write(sockid, offset, length, data);
}

void udp_tx_flush(net_socket_t sockid)
{
    socket_tx_flush(sockid);
}

void udp_send(net_socket_t sockid, const uint8_t *destip, uint16_t destport, uint16_t length, const uint8_t *data)
{
    if (!udp_tx_prepare(sockid, destip, destport))
        return;

    udp_tx_write(sockid, 0, length, data);
    udp_tx_flush(sockid);
}

int16_t udp_available(net_socket_t sockid)
{
    uint16_t ret = socket_rx_rsr(sockid);
    if (ret >= sizeof(struct udp_w5100_header)) // If a packet is available, read its size.
    {
        struct udp_w5100_header header;
        socket_rx_read(sockid, 0, sizeof(struct udp_w5100_header), (uint8_t*)&header); // Don't call flush. We only want the data length 
        ret = header.length;
    }
    else
        ret = 0;
    return ret;
}

uint16_t udp_recv(net_socket_t sockid, uint16_t bufsize, uint8_t *data)
{
    uint16_t packet_length = 0;
    uint16_t get = 0;
    uint16_t count = 0;


    packet_length = udp_available(sockid);
    
    if (!packet_length)
        return packet_length;

    get = MIN(packet_length, bufsize);
    count = socket_rx_read(sockid, sizeof(struct udp_w5100_header), get, data);
    socket_rx_flush(sockid, sizeof(struct udp_w5100_header) + packet_length); // Flush both the header and the packet
    return count;
}

