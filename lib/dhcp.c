#include "dhcp.h"
#include "w5100.h"
#include "socket.h"
#include "udp.h"
#include "util.h"
#include "timer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const uint8_t dhcp_broadcast[] = {255, 255, 255, 255};
static uint8_t dhcp_server[] = {0, 0, 0, 0};
static uint32_t dhcp_xid = 0;
static uint16_t dhcp_start_time = 0;

uint8_t dhcp_get_ip()
{

    uint8_t sockid = SOCKET_INVALID;

    // First get a socket
    sockid = udp_open(DHCP_CLIENT_PORT);
    if (sockid == SOCKET_INVALID)
        return 1;
    dhcp_send_packet(sockid, DHCP_DISCOVER);
    if (dhcp_read_packet(sockid) != DHCP_OFFER)
        return 2;
    dhcp_send_packet(sockid, DHCP_REQUEST);
    if (dhcp_read_packet(sockid) != DHCP_ACK)
        return 3;
    
    // Finally close the socket
    udp_close(sockid);
    return 0;
}

void dhcp_send_packet(uint8_t sockid, uint8_t msgtype)
{
    udp_tx_prepare(sockid, dhcp_broadcast, DHCP_SERVER_PORT);

#if DHCP_USE_MALLOC
    uint8_t *buffer = malloc(32);
#else
    uint8_t buffer[32];
#endif
    net_offset_t offset = 0;

    memset(buffer, 0, 32);

    buffer[0] = DHCP_OP_REQUEST;
    buffer[1] = DHCP_HTYPE_ETHERNET;
    buffer[2] = DHCP_HLEN_ETHERNET;
    buffer[3] = 0;
    uint32_t xid = util_htonl(dhcp_xid);
    memcpy(buffer + 4, &xid, 4);
    uint16_t secs = util_htons(timer_secs() - dhcp_start_time);
    memcpy(buffer + 8, &secs, 2);
    uint16_t flags = util_htons(DHCP_FLAGS_BROADCAST);
    memcpy(buffer + 10, &flags, 2);

    // All ip addresses fields have been reset by memset
    udp_tx_write(sockid, offset, 28, buffer);
    offset += 28; // We have written 28 bytes so far

    w5100_get_hwaddr(buffer);
    udp_tx_write(sockid, offset, 16, buffer); // Even though the mac address is 4bytes, the chaddr field is 16
    offset += 16;

    // Now clear th sname and file fields. They are 192 bytes in total
    // This is 6*32 bytes so send 6 empty buffers
    memset(buffer, 0, 32);
    for (int i = 0; i < 6; i++)
    {
        udp_tx_write(sockid, offset, 32, buffer);
        offset += 32;
    }

    uint32_t magic = util_htonl(DHCP_MAGIC);
    memcpy(buffer, &magic, 4);
    buffer[4] = DHCP_MSGTYPE;
    buffer[5] = 1;
    buffer[6] = msgtype;

    udp_tx_write(sockid, offset, 7, buffer);
    offset += 7;

    switch (msgtype)
    {
    case DHCP_REQUEST:
        buffer[0] = DHCP_REQ_IPADDR;
        buffer[1] = 4;
        w5100_get_ipaddr(buffer + 2);

        buffer[6] = DHCP_SERVER_ID;
        buffer[7] = 4;
        memcpy(buffer + 8, dhcp_server, 4);

        udp_tx_write(sockid, offset, 12, buffer);
        offset += 12;

        break;
    default:
        break;
    }

    buffer[0] = DHCP_END;
    udp_tx_write(sockid, offset, 1, buffer);
    offset += 1;

#if DHCP_USE_MALLOC
    free(buffer);
#endif

    printf("Sending DHCP packet ..\n\r");
    udp_tx_flush(sockid);
}

uint8_t dhcp_read_packet(uint8_t sockid)
{
//    uint16_t start_time = 0;
    uint8_t type = 0;

    net_size_t data_len = -1;
    do {
        if (data_len >= 0) // That means the packet was rejected by the while
        {
            udp_tx_flush(sockid);
        }
        data_len = udp_rx_available(sockid);
    } while (data_len < DHCP_OPTIONS_OFFSET); // Minimum dhcp message
    
#if DHCP_USE_MALLOC
    struct dhcp_header *header = malloc(sizeof(struct dhcp_header));
#else
    struct dhcp_header _header;
    struct dhcp_header *header = &_header;
#endif

    udp_rx_read(sockid, 0, sizeof(struct dhcp_header), (uint8_t *)header);

    uint8_t hwaddr[6];
    w5100_get_hwaddr(hwaddr);

    if (header->op == DHCP_OP_REPLY 
        && util_ntohl(header->xid) == dhcp_xid
        && memcmp(header->chaddr,hwaddr, 6) == 0)
    {
        type = 0xff;
        w5100_set_ipaddr(header->yiaddr);
        net_size_t option_len = data_len - DHCP_OPTIONS_OFFSET;
        uint8_t *buffer = malloc(option_len);
        udp_rx_read(sockid, DHCP_OPTIONS_OFFSET, option_len, buffer);

        uint8_t *p = buffer;
        uint8_t *e = p + option_len;

        
        while (p < e)
        {
            switch (p[0])
            {
            case DHCP_END:
                p = e; // End the loop
                break;
            case DHCP_MSGTYPE:
                type = p[2];
                break;
            case DHCP_SUBNET:
                w5100_set_subnet(p + 2);
                break;
            case DHCP_ROUTER:
                w5100_set_gateway(p + 2);
                break;
            case DHCP_SERVER_ID:
                memcpy(dhcp_server, p + 2, 4);
                break;
            default:
               break;
            }
            
            if (p[0] != DHCP_END) // The end do not have a length option
                p += p[1] + 2;
        }

        free(buffer);
    }
#if DHCP_USE_MALLOC
    free(header);
#endif
    udp_rx_flush(sockid);
    return type;
}
