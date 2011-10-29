#include "socket.h"
#include "w5100.h"
#include "util.h"

uint8_t socket_get_type(uint8_t sockid)
{
    return w5100_read(W5100_SOCKET(sockid) + W5100_SOCK_MR) & W5100_SOCK_MR_P_MSK;
}

void socket_set_type(uint8_t sockid, uint8_t type)
{
    w5100_write_and(W5100_SOCKET(sockid) + W5100_SOCK_MR, ~W5100_SOCK_MR_P_MSK); // Turn off all type bits
    w5100_write_or(W5100_SOCKET(sockid) + W5100_SOCK_MR, type << W5100_SOCK_MR_P); // And set the corresponding ones.
}

uint8_t socket_alloc(uint8_t type)
{
#if SOCKET_SOCK0_RESERVED
    // If sock 0 is required start with this one, else start with socket 1 to keep 0 for later ..
    int i = (type >= SOCKET_TYPE_SOCK0)?0:1;
#else
    // Don't reserve socket 0.
    int i = 0; 
#endif

    for (;i < ((type >= SOCKET_TYPE_SOCK0)?1:SOCKET_SOCK_COUNT); i++)
    {
        if(socket_get_type(i) == SOCKET_TYPE_CLOSED)
        {
            socket_set_type(i, type);
            return i;
        }
    }
    return SOCKET_INVALID;
}

void socket_free(uint8_t sockid)
{
    if (sockid != SOCKET_INVALID)
        socket_set_type(sockid, SOCKET_TYPE_CLOSED);
}

void socket_cmd(uint8_t sockid, uint8_t cmd)
{
    w5100_write(W5100_SOCKET(sockid) + W5100_SOCK_CR, cmd);
}

uint8_t socket_status(uint8_t sockid)
{
    return w5100_read(W5100_SOCKET(sockid) + W5100_SOCK_SR);
}


uint8_t socket_ir(uint8_t sockid)
{
    return w5100_read(W5100_SOCKET(sockid) + W5100_SOCK_IR);
}

void socket_ir_clear(uint8_t sockid, uint8_t ir)
{
    w5100_write(W5100_SOCKET(sockid) + W5100_SOCK_IR, ir);
}


void socket_set_port(uint8_t sockid, uint16_t port)
{
    w5100_write16(W5100_SOCKET(sockid) + W5100_SOCK_PORT, port);
}

void socket_set_dest_ip(uint8_t sockid, const uint8_t *destip)
{
    w5100_write_array(W5100_SOCKET(sockid) + W5100_SOCK_DIPR, 4, destip);
}

void socket_set_dest_port(uint8_t sockid, uint16_t destport)
{
    w5100_write16(W5100_SOCKET(sockid) + W5100_SOCK_DPORT, destport);
}


uint8_t socket_tx_prepare(uint8_t sockid)
{
    if (socket_tx_rd(sockid) != socket_tx_wr(sockid)) // Data is pending
        return 0;

    return 1;
}

void socket_tx_write(uint8_t sockid, uint16_t write_offset, uint16_t length, const uint8_t *data)
{
    while(length > socket_tx_fsr(sockid)) // Wait for free space
    {
    }

    uint16_t offset = socket_tx_wr(sockid) + write_offset;
    uint16_t start = SOCKET_TX_BASE_S(sockid)+ offset;

    if (start + length >= SOCKET_TX_END_S(sockid)) // If we need to split it up ..
    {
        uint16_t first_size = SOCKET_TX_END_S(sockid) - start;
        uint16_t second_size = length - first_size;

        w5100_write_array(start, first_size, data);
        w5100_write_array(SOCKET_TX_BASE_S(sockid), second_size, data + first_size);
    }
    else
    {
        w5100_write_array(start, length, data);
    }

    socket_tx_set_wr(sockid, offset + length);
    w5100_write16(W5100_SOCKET(sockid) + W5100_SOCK_TX_WR, offset + length);
}

void socket_tx_flush(uint8_t sockid)
{
    socket_cmd(sockid, SOCKET_CMD_SEND);

    // Wait for sending to be complete
    while (!(socket_ir(sockid) & W5100_SOCK_IR_SEND_OK))
    {
    }
    // Reset the SEND_OK interrupt
    socket_ir_clear(sockid, W5100_SOCK_IR_SEND_OK);
}


uint16_t socket_tx_fsr(uint8_t sockid)
{
    return w5100_read16(W5100_SOCKET(sockid) + W5100_SOCK_TX_FSR);
}

uint16_t socket_tx_wr(uint8_t sockid)
{
    return w5100_read16(W5100_SOCKET(sockid) + W5100_SOCK_TX_WR) & SOCKET_TX_MASK_S(sockid);
}

void socket_tx_set_wr(uint8_t sockid, uint16_t wr)
{
    w5100_write16(W5100_SOCKET(sockid) + W5100_SOCK_TX_WR, wr);
}

uint16_t socket_tx_rd(uint8_t sockid)
{
    return w5100_read16(W5100_SOCKET(sockid) + W5100_SOCK_TX_RD) & SOCKET_TX_MASK_S(sockid);
}


uint16_t socket_rx_read(uint8_t sockid, uint16_t read_offset, uint16_t count, uint8_t *data)
{
    uint16_t available = socket_rx_rsr(sockid);
    uint16_t length = MIN(available, count);

    uint16_t offset = socket_rx_rd(sockid) + read_offset;
    uint16_t start = offset + SOCKET_RX_BASE_S(sockid); 

    if (start + length >= SOCKET_RX_END_S(sockid)) // If we need to split it up ..
    {
        uint16_t first_size = SOCKET_RX_END_S(sockid) - start;
        uint16_t second_size = length - first_size;

        w5100_read_array(start, first_size, data);
        w5100_read_array(SOCKET_RX_BASE_S(sockid), second_size, data + first_size);
    }
    else
    {
        w5100_read_array(start, length, data);
    }

    return length;
}

uint16_t socket_rx_flush(uint8_t sockid, uint16_t count)
{
    uint16_t available = socket_rx_rsr(sockid);
    uint16_t length = MIN(available, count);
    uint16_t offset = socket_rx_rd(sockid);

    socket_rx_set_rd(sockid, offset + length);
    socket_cmd(sockid, SOCKET_CMD_RECV);

    return length;
}

uint16_t socket_rx_rsr(uint8_t sockid)
{
    return w5100_read16(W5100_SOCKET(sockid) + W5100_SOCK_RX_RSR);
}

uint16_t socket_rx_rd(uint8_t sockid)
{
    return w5100_read16(W5100_SOCKET(sockid) + W5100_SOCK_RX_RD) & SOCKET_RX_MASK_S(sockid);
}

void socket_rx_set_rd(uint8_t sockid, uint16_t rd)
{
    w5100_write16(W5100_SOCKET(sockid) + W5100_SOCK_RX_RD, rd);
}


