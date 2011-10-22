#ifndef _dhcp_h__
#define _dhcp_h__

#include <stdint.h>
// Information about DHCP was taken from
// http://www.frameip.com/dhcp/
// http://www.ietf.org/rfc/rfc2131.txt
// http://www.ietf.org/rfc/rfc2132.txt

#define DHCP_CLIENT_PORT 68
#define DHCP_SERVER_PORT 67

struct dhcp_packet
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
    uint8_t sname[64];
    uint8_t file[128];
    uint32_t magic;
    uint8_t options[336];
} __attribute((packed));

#define DHCP_OP_REQUEST 1
#define DHCP_OP_REPLY 2

#define DHCP_HTYPE_ETHERNET 1 // There are many others, but we don't use them

#define DHCP_HLEN_ETHERNET 6 

#define DHCP_XID (rand() / (RAND_MAX / UINT32_MAX + 1)) // Could be replaced by fixed number

#define DHCP_SECS 0

#define DHCP_FLAGS_BROADCAST (1<<15)

#define DHCP_MAGIC 0x63538263

#define DHCP_OPTION_MSGTYPE 53
#define DHCP_OPTION_MSGTYPE_SIZE 1
#define DHCP_OPTION_MSGTYPE_DISCOVER 1
#define DHCP_OPTION_MSGTYPE_OFFER 2
#define DHCP_OPTION_MSGTYPE_REQUEST 3
#define DHCP_OPTION_MSGTYPE_DECLINE 4
#define DHCP_OPTION_MSGTYPE_ACK 5
#define DHCP_OPTION_MSGTYPE_NAK 6
#define DHCP_OPTION_MSGTYPE_RELEASE 6
#define DHCP_OPTION_MSGTYPE_INFORM 7

#define DHCP_OPTION_MAXLENGTH 57
#define DHCP_OPTION_MAXLENGTH_SIZE 2

#define DHCP_OPTION_SUBNET 1
#define DHCP_OPTION_SUBNET_SIZE 4
#define DHCP_OPTION_ROUTER 3
#define DHCP_OPTION_ROUTER_SIZE 4

#define DHCP_OPTION_REQ_IPADDR 50
#define DHCP_OPTION_REQ_IPADDR_SIZE 4

#define DHCP_OPTION_SERVER_ID 54
#define DHCP_OPTION_SERVER_ID_SIZE 4

#define DHCP_OPTION_END 255

uint8_t dhcp_get_ip(uint8_t *ip);

void dhcp_build_packet(struct dhcp_packet *packet, uint32_t xid);
void dhcp_send_discover(uint8_t sockid, struct dhcp_packet *packet, uint32_t xid);
void dhcp_send_request(uint8_t sockid, struct dhcp_packet *packet, const uint8_t *server, uint32_t xid);

uint8_t dhcp_packet_type(const struct dhcp_packet *packet);
uint8_t dhcp_parse_offer(const struct dhcp_packet *packet, uint32_t xid, uint8_t *server);
uint8_t dhcp_parse_ack(const struct dhcp_packet *packet, uint32_t xid);

uint8_t *dhcp_option_cmdtype(uint8_t *option_ptr, uint8_t type);
uint8_t *dhcp_option_maxlength(uint8_t *option_ptr, uint16_t maxlength);
uint8_t *dhcp_option_requested_ipaddr(uint8_t *option_ptr);
uint8_t *dhcp_option_server_id(uint8_t *option_ptr, const uint8_t *server);
uint8_t *dhcp_option_end(uint8_t *option_ptr);

#endif
