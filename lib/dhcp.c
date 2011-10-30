#include "dhcp.h"
#include "w5100.h"
#include "socket.h"
#include "udp.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

static uint8_t broadcast[] = {255, 255, 255, 255};

uint8_t dhcp_get_ip()
{
    static struct dhcp_packet packet;
    static uint8_t server[4];

    uint8_t sockid = SOCKET_INVALID;
    uint32_t xid = DHCP_XID; // XID should be the same during the hole transmission

    // First get a socket
    sockid = udp_open(DHCP_CLIENT_PORT);
    if (sockid == SOCKET_INVALID)
        return 0;

    dhcp_send_discover(sockid, &packet, xid);

    while(1)
    {
        if (udp_rx_available(sockid) >= 0 )
        {
            udp_rx_read(sockid, 0, sizeof(packet), (uint8_t *)&packet);
            udp_rx_flush(sockid);
            if (dhcp_parse_offer(&packet, xid, server))
                break;
        }
    }

    dhcp_send_request(sockid, &packet, server, xid);

    while(1)
    {
        if (udp_rx_available(sockid) > 0 )
        {
            udp_rx_read(sockid, 0, sizeof(packet), (uint8_t *)&packet);
            udp_rx_flush(sockid);
            if (dhcp_parse_ack(&packet, xid))
                break;
        }
    }

    // Finally close the socket
    udp_close(sockid);
    return 1;
}

void dhcp_build_packet(struct dhcp_packet *packet, uint32_t xid)
{
    memset(packet, 0, sizeof(struct dhcp_packet));

    packet->op = DHCP_OP_REQUEST;
    packet->htype = DHCP_HTYPE_ETHERNET;
    packet->hlen = DHCP_HLEN_ETHERNET;
    packet->hops = 0;
    packet->xid = util_htonl(xid);
    packet->secs = util_htons(DHCP_SECS);
    packet->flags = util_htons(0);//DHCP_FLAGS_BROADCAST); // For debug, else use 0
    // All ip address fields have been reset by memset
    w5100_get_hwaddr(packet->chaddr);
    // sname and file have been reset too
    packet->magic = DHCP_MAGIC;
}

void dhcp_send_discover(uint8_t sockid, struct dhcp_packet *packet, uint32_t xid)
{
    uint8_t *option_ptr = 0;
    // Setup the packet
    dhcp_build_packet(packet, xid);
    option_ptr = packet->options;
    option_ptr = dhcp_option_cmdtype(option_ptr, DHCP_OPTION_MSGTYPE_DISCOVER);
    option_ptr = dhcp_option_maxlength(option_ptr, sizeof(struct dhcp_packet));
    option_ptr = dhcp_option_end(option_ptr);

    udp_tx_prepare(sockid, broadcast, DHCP_SERVER_PORT);
    udp_tx_write(sockid, 0, sizeof(struct dhcp_packet), (uint8_t *)packet);
    udp_tx_flush(sockid);
}

void dhcp_send_request(uint8_t sockid, struct dhcp_packet *packet, const uint8_t *server, uint32_t xid)
{
    uint8_t *option_ptr = 0;
    // Setup the packet
    dhcp_build_packet(packet, xid);
    option_ptr = packet->options;
    option_ptr = dhcp_option_cmdtype(option_ptr, DHCP_OPTION_MSGTYPE_REQUEST);
    option_ptr = dhcp_option_maxlength(option_ptr, sizeof(struct dhcp_packet));
    option_ptr = dhcp_option_requested_ipaddr(option_ptr);
    option_ptr = dhcp_option_server_id(option_ptr, server);
    option_ptr = dhcp_option_end(option_ptr);

    udp_tx_prepare(sockid, broadcast, DHCP_SERVER_PORT);
    udp_tx_write(sockid, 0, sizeof(struct dhcp_packet), (uint8_t *)packet);
    udp_tx_flush(sockid);
}

uint8_t dhcp_packet_type(const struct dhcp_packet *packet)
{
    for (const uint8_t *option_ptr = packet->options; option_ptr[0] != DHCP_OPTION_END; option_ptr += option_ptr[1] + 2)
    {
        switch (option_ptr[0])
        {
            case DHCP_OPTION_MSGTYPE:
                return option_ptr[2];
                break;
            default:
                break;
        }
    }
    return 0;
}

uint8_t dhcp_parse_offer(const struct dhcp_packet *packet, uint32_t xid, uint8_t *server)
{
    if (packet->op != DHCP_OP_REPLY
            || util_ntohl(packet->xid) != xid
            || packet->magic != DHCP_MAGIC
            || dhcp_packet_type(packet) != DHCP_OPTION_MSGTYPE_OFFER)
        return 0;

    memset(server, 0, 4);
    w5100_set_ipaddr(packet->yiaddr);

    for (const uint8_t *option_ptr = packet->options; option_ptr[0] != DHCP_OPTION_END; option_ptr += option_ptr[1] + 2)
    {
        switch (option_ptr[0])
        {
            case DHCP_OPTION_ROUTER:
                w5100_set_gateway(option_ptr + 2);
                break;
            case DHCP_OPTION_SUBNET:
                w5100_set_subnet(option_ptr + 2);
                break;
            case DHCP_OPTION_SERVER_ID:
                memcpy(server, option_ptr + 2, 4);
                break;
            default:
                break;
        }
    }
    return 1;
}

uint8_t dhcp_parse_ack(const struct dhcp_packet *packet, uint32_t xid)
{
    if (packet->op != DHCP_OP_REPLY
            || util_ntohl(packet->xid) != xid
            || packet->magic != DHCP_MAGIC
            || dhcp_packet_type(packet) != DHCP_OPTION_MSGTYPE_ACK)
        return 0;
    return 1;
}

uint8_t *dhcp_option_cmdtype(uint8_t *option_ptr, uint8_t type)
{
    option_ptr[0] = DHCP_OPTION_MSGTYPE;
    option_ptr[1] = DHCP_OPTION_MSGTYPE_SIZE;
    option_ptr[2] = type;
    option_ptr += DHCP_OPTION_MSGTYPE_SIZE + 2;
    return option_ptr;
}

uint8_t *dhcp_option_maxlength(uint8_t *option_ptr, uint16_t maxlength)
{
    option_ptr[0] = DHCP_OPTION_MAXLENGTH;
    option_ptr[1] = DHCP_OPTION_MAXLENGTH_SIZE;
    option_ptr[2] = (maxlength & 0xFF00) >> 8;
    option_ptr[3] = (maxlength & 0x00FF) >> 0;

    option_ptr += DHCP_OPTION_MAXLENGTH_SIZE + 2;
    return option_ptr; 
}

uint8_t *dhcp_option_requested_ipaddr(uint8_t *option_ptr)
{
    option_ptr[0] = DHCP_OPTION_REQ_IPADDR;
    option_ptr[1] = DHCP_OPTION_REQ_IPADDR_SIZE;
    w5100_get_ipaddr(option_ptr + 2);

    option_ptr += DHCP_OPTION_REQ_IPADDR_SIZE + 2;
    return option_ptr;
}

uint8_t *dhcp_option_server_id(uint8_t *option_ptr, const uint8_t *server)
{
    option_ptr[0] = DHCP_OPTION_SERVER_ID;
    option_ptr[1] = DHCP_OPTION_SERVER_ID_SIZE;
    memcpy(option_ptr + 2, server, 4);

    option_ptr += DHCP_OPTION_REQ_IPADDR_SIZE + 2;
    return option_ptr;
}

uint8_t *dhcp_option_end(uint8_t *option_ptr)
{
    option_ptr[0] = DHCP_OPTION_END;
    return option_ptr;
}


