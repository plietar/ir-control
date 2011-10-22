#ifndef _w5100_h__
#define _w5100_h__

#include <stdint.h>

#define W5100_SS_DDR DDRB
#define W5100_SS_PORT PORTB
#define W5100_SS_BV _BV(2)

#define W5100_SELECT() W5100_SS_PORT &= ~W5100_SS_BV
#define W5100_DESELECT() W5100_SS_PORT |= W5100_SS_BV

#define W5100_OPCODE_WRITE 0xF0
#define W5100_OPCODE_READ  0x0F

#define W5100_MR 0x00

#define W5100_GAR 0x01
#define W5100_SUBR 0x05
#define W5100_SHAR 0x09
#define W5100_SIPR 0x0F

#define W5100_IR 0x15
#define W5100_IRM 0x16
#define W5100_RTR 0x17
#define W5100_RCR 0x19
#define W5100_RMSR 0x1A
#define W5100_TMSR 0x1B
#define W5100_PATR 0x1C

#define W5100_PTIMER 0x28
#define W5100_PMAGIC 0x29

#define W5100_UIPR 0x2A
#define W5100_UPORT 0x2E

#define W5100_MR_RST 0x7
#define W5100_MR_PB 0x4
#define W5100_MR_PPPOE 0x3
#define W5100_MR_AI 0x1
#define W5100_MR_IND 0x0

#define W5100_SOCKET(sockid) (0x400 + (sockid) * 0x100)
#define W5100_SOCK_MR 0x00
#define W5100_SOCK_CR 0x01
#define W5100_SOCK_IR 0x02
#define W5100_SOCK_SR 0x03
#define W5100_SOCK_PORT 0x04
#define W5100_SOCK_DHAR 0x06
#define W5100_SOCK_DIPR 0x0C
#define W5100_SOCK_DPORT 0x10
#define W5100_SOCK_MSSR 0x12
#define W5100_SOCK_PROTO 0x14
#define W5100_SOCK_TOS 0x15
#define W5100_SOCK_TTL 0x16
#define W5100_SOCK_TX_FSR 0x20
#define W5100_SOCK_TX_RD 0x22
#define W5100_SOCK_TX_WR 0x24
#define W5100_SOCK_RX_RSR 0x26
#define W5100_SOCK_RX_RD 0x28

#define W5100_SOCK_MR_P 0x0
#define W5100_SOCK_MR_P_MSK 0x0F

#define W5100_SOCK_IR_SEND_OK 1<<4
#define W5100_SOCK_IR_TIMEOUT 1<<3
#define W5100_SOCK_IR_RECV 1<<2
#define W5100_SOCK_IR_DISCON 1<<1
#define W5100_SOCK_IR_CON 1<<0

void w5100_init();

void w5100_init_no_spi();

uint8_t w5100_read(uint16_t addr, uint8_t *out);

uint16_t w5100_read16(uint16_t addr, uint16_t *out);

uint8_t *w5100_read_array(uint16_t addr, uint16_t count, uint8_t *out);

void w5100_write(uint16_t addr, uint8_t data);

void w5100_write_or(uint16_t addr, uint8_t data);

void w5100_write_and(uint16_t addr, uint8_t data);

void w5100_write_xor(uint16_t addr, uint8_t data);

void w5100_write16(uint16_t addr, uint16_t data);

void w5100_write16_add(uint16_t addr, int16_t data);

void w5100_write_array(uint16_t start, uint16_t count, const uint8_t *data);

void w5100_set_hwaddr(const uint8_t *hwaddr);
void w5100_get_hwaddr(uint8_t *hwaddr);

void w5100_set_ipaddr(const uint8_t *ipaddr);
void w5100_get_ipaddr(uint8_t *ipaddr);

void w5100_set_subnet(const uint8_t *subnet);
void w5100_get_subnet(uint8_t *subnet);

void w5100_set_gateway(const uint8_t *gateway);
void w5100_get_gateway(uint8_t *gateway);


#endif
