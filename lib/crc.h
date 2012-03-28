#ifndef _crc_h__
#define _crc_h__

#include <stdint.h>
#include <stdlib.h>

uint32_t crc32_update(uint32_t crc, uint8_t data);
uint32_t crc32_string(const char *s);
uint32_t crc32_bytes(const uint8_t *b, size_t len);

uint16_t crc16_bytes(const uint8_t *b, size_t len);

#endif // _crc_h__
