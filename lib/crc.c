#include "crc.h"
#include <avr/pgmspace.h>
#include <util/crc16.h>
#include <stdint.h>

static const PROGMEM uint32_t crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

uint32_t crc32_update(uint32_t crc, uint8_t data)
{
    uint8_t tbl_idx;
    tbl_idx = crc ^ (data >> (0 * 4));
    crc = pgm_read_dword_near(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
    tbl_idx = crc ^ (data >> (1 * 4));
    crc = pgm_read_dword_near(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
    return crc;
}

uint32_t crc32_string(const char *s)
{
    uint32_t crc = ~0L;
    while (*s)
        crc = crc32_update(crc, *s++);
    crc = ~crc;
    return crc;
}

uint32_t crc32_bytes(const uint8_t *b, size_t len)
{
    uint32_t crc = ~0L;
    for (unsigned int i = 0; i < len; i++)
    {
        crc = crc32_update(crc, b[i]);
    }
    crc = ~crc;
    return crc;
}

uint16_t crc16_bytes(const uint8_t *b, size_t len)
{
    uint16_t crc = 0xFFFF;
    for (unsigned int i = 0; i < len; i++)
    {
        crc = _crc16_update(crc, b[i]);
    }
    crc = ~crc;
    return crc;
}
