#ifndef _util_h__
#define _util_h__

#include <stdint.h>

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

uint16_t util_htons(unsigned short hostshort);
uint32_t util_htonl(unsigned long hostlong);
uint16_t util_ntohs(unsigned short netshort);
uint32_t util_ntohl(unsigned long netlong);
uint16_t util_swaps(uint16_t i);
uint32_t util_swapl(uint32_t l);

#endif
