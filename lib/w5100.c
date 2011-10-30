#include "spi.h"
#include "w5100.h"
#include <util/delay.h>
#include <stddef.h>

void w5100_init()
{
    W5100_SS_DDR |= W5100_SS_BV;
    spi_init(SPI_MODE_0, SPI_MSB, SPI_NO_INTERRUPT, SPI_MSTR_CLK2);
    
    w5100_init_no_spi();
}

void w5100_init_no_spi()
{
    w5100_write(W5100_MR, 1 << W5100_MR_RST); // Reset the W5100
    _delay_ms(10); // Let it breathe a little
    
    w5100_write(W5100_MR, 0); //Disable ping block, PPPoE and Indirect mode

    // Set up buffers as 4 * 2KB.
    // Should be the default but just to make sure
    w5100_write(W5100_RMSR, 0x55);
    w5100_write(W5100_TMSR, 0x55);
}

uint8_t w5100_read(net_addr_t addr)
{
    W5100_SELECT();
    uint8_t data;
    spi_transfer(W5100_OPCODE_READ); // Start read sequence
    spi_transfer((addr & 0xFF00) >> 8); // Send Address MSB 
    spi_transfer((addr & 0x00FF)); // Send Address LSB
    data = spi_transfer(0x00); // And read the data
    W5100_DESELECT();
    return data;
}

uint16_t w5100_read16(net_addr_t addr)
{
    uint16_t data = 0;
    uint8_t high = w5100_read(addr);
    uint8_t low = w5100_read(addr + 1);
    data = ( high << 8 | low );
    return data;
}

uint8_t *w5100_read_array(net_addr_t addr, net_size_t count, uint8_t *out)
{
    for(net_size_t i = 0; i < count; i++)
    {
        out[i] = w5100_read(addr + i);
    }

    return out;
}

void w5100_write(net_addr_t addr, uint8_t data)
{
    W5100_SELECT();
    spi_transfer(W5100_OPCODE_WRITE); // Start write sequence
    spi_transfer((addr & 0xFF00) >> 8); // Send Address MSB 
    spi_transfer((addr & 0x00FF)); // Send Address LSB
    spi_transfer(data); // And send the data
    W5100_DESELECT();
}

void w5100_write_or(net_addr_t addr, uint8_t data)
{
    w5100_write(addr, w5100_read(addr) | data);
}

void w5100_write_and(net_addr_t addr, uint8_t data)
{
    w5100_write(addr, w5100_read(addr) & data);
}

void w5100_write_xor(net_addr_t addr, uint8_t data)
{
    w5100_write(addr, w5100_read(addr) ^ data);
}

void w5100_write16(net_addr_t addr, uint16_t data)
{
    w5100_write(addr, (data & 0xFF00) >> 8);
    w5100_write(addr +1, data & 0x00FF);
}

void w5100_write16_add(net_addr_t addr, uint16_t data)
{
    w5100_write16(addr, w5100_read16(addr) + data);
}

void w5100_write_array(net_addr_t addr, net_size_t count, const uint8_t *data)
{
    for (net_size_t i = 0; i < count; i++)
    {
        w5100_write(addr + i, data[i]);
    }
}

void w5100_set_hwaddr(const uint8_t *hwaddr)
{
    w5100_write_array(W5100_SHAR, 6, hwaddr);
}

void w5100_get_hwaddr(uint8_t *hwaddr)
{
    w5100_read_array(W5100_SHAR, 6, hwaddr);
}

void w5100_set_ipaddr(const uint8_t *ipaddr)
{
    w5100_write_array(W5100_SIPR, 4, ipaddr);
}

void w5100_get_ipaddr(uint8_t *ipaddr)
{
    w5100_read_array(W5100_SIPR, 4, ipaddr);
}

void w5100_set_subnet(const uint8_t *subnet)
{
    w5100_write_array(W5100_SUBR, 4, subnet);
}

void w5100_get_subnet(uint8_t *subnet)
{
    w5100_read_array(W5100_SUBR, 4, subnet);
}

void w5100_set_gateway(const uint8_t *gateway)
{
    w5100_write_array(W5100_GAR, 4, gateway);
}

void w5100_get_gateway(uint8_t *gateway)
{
    w5100_read_array(W5100_GAR, 4, gateway);
}


