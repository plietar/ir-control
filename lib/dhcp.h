#ifndef _dhcp_h__
#define _dhcp_h__

#include <stdint.h>
// Information about DHCP was taken from
// http://www.frameip.com/dhcp/
// http://www.ietf.org/rfc/rfc2131.txt
// http://www.ietf.org/rfc/rfc2132.txt

#define DHCP_CLIENT_PORT 68
#define DHCP_SERVER_PORT 67

struct dhcp_header
{
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    
    uint32_t xid;

    uint16_t secs;
    uint16_t flags;

    uint8_t ciaddr[4];
    uint8_t yiaddr[4];
    uint8_t siaddr[4];
    uint8_t giaddr[4];
    uint8_t chaddr[16];
} __attribute((packed));

#define DHCP_MAGIC_OFFSET 236
#define DHCP_OPTIONS_OFFSET 240

#define DHCP_OP_REQUEST 1
#define DHCP_OP_REPLY 2

#define DHCP_HTYPE_ETHERNET 1 // There are many others, but we don't use them

#define DHCP_HLEN_ETHERNET 6 

#define DHCP_FLAGS_BROADCAST (1<<15)

#define DHCP_MAGIC 0x63825363

#define DHCP_MSGTYPE 53
#define DHCP_DISCOVER 1
#define DHCP_OFFER 2
#define DHCP_REQUEST 3
#define DHCP_DECLINE 4
#define DHCP_ACK 5
#define DHCP_NAK 6
#define DHCP_RELEASE 6
#define DHCP_INFORM 7

#define DHCP_MAXLENGTH 57

#define DHCP_SUBNET 1
#define DHCP_ROUTER 3
#define DHCP_REQ_IPADDR 50
#define DHCP_SERVER_ID 54

#define DHCP_END 255

#define DHCP_USE_MALLOC 1

uint8_t dhcp_get_ip();

void dhcp_send_packet(uint8_t sockid, uint8_t msgtype);
uint8_t dhcp_read_packet(uint8_t sockid);

#endif
