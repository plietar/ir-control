#include "socket.h"
#include "w5100.h"
#include "util.h"
#include <stdio.h>

uint8_t socket_get_type(net_socket_t sockid)
{
    return w5100_read(W5100_SOCKET(sockid) + W5100_SOCK_MR) & W5100_SOCK_MR_P_MSK;
}

void socket_set_type(net_socket_t sockid, uint8_t type)
{
    w5100_write_and(W5100_SOCKET(sockid) + W5100_SOCK_MR, ~W5100_SOCK_MR_P_MSK); // Turn off all type bits
    w5100_write_or(W5100_SOCKET(sockid) + W5100_SOCK_MR, type << W5100_SOCK_MR_P); // And set the corresponding ones.
}

net_socket_t socket_alloc(uint8_t type)
{
#if SOCKET_SOCK0_RESERVED
    // If sock 0 is required start with this one, else start with socket 1 to keep 0 for later ..
    net_socket_t i = (type >= SOCKET_TYPE_SOCK0)?0:1;
#else
    // Don't reserve socket 0.
    net_socket_t i = 0;
#endif

    for (;i < ((type >= SOCKET_TYPE_SOCK0)?1:SOCKET_SOCK_COUNT); i++)
    {
        if(socket_get_type(i) == SOCKET_TYPE_CLOSED || socket_status(i) == SOCKET_STATUS_CLOSED)
        {
            socket_set_type(i, type);
            return i;
        }
    }
    return SOCKET_INVALID;
}

void socket_free(net_socket_t sockid)
{
    if (sockid != SOCKET_INVALID)
        socket_set_type(sockid, SOCKET_TYPE_CLOSED);
}

void socket_close(net_socket_t sockid)
{
    if (sockid != SOCKET_INVALID)
    {
        socket_cmd(sockid, SOCKET_CMD_DISCON);
        socket_free(sockid);
    }
}

void socket_disconnect(net_socket_t sockid)
{
    if (sockid != SOCKET_INVALID)
    {
        socket_cmd(sockid, SOCKET_CMD_DISCON);
        socket_free(sockid);
    }
}

void socket_cmd(net_socket_t sockid, uint8_t cmd)
{
    w5100_write(W5100_SOCKET(sockid) + W5100_SOCK_CR, cmd);
}

uint8_t socket_status(net_socket_t sockid)
{
    return w5100_read(W5100_SOCKET(sockid) + W5100_SOCK_SR);
}

uint8_t socket_ir(net_socket_t sockid)
{
    return w5100_read(W5100_SOCKET(sockid) + W5100_SOCK_IR);
}

void socket_ir_clear(net_socket_t sockid, uint8_t ir)
{
    w5100_write(W5100_SOCKET(sockid) + W5100_SOCK_IR, ir);
}

uint16_t socket_port(net_socket_t sockid)
{
    return w5100_read16(W5100_SOCKET(sockid) + W5100_SOCK_PORT);
}

void socket_set_port(net_socket_t sockid, uint16_t port)
{
    w5100_write16(W5100_SOCKET(sockid) + W5100_SOCK_PORT, port);
}

void socket_set_dest_ip(net_socket_t sockid, const uint8_t *destip)
{
    w5100_write_array(W5100_SOCKET(sockid) + W5100_SOCK_DIPR, 4, destip);
}

void socket_set_dest_port(net_socket_t sockid, uint16_t destport)
{
    w5100_write16(W5100_SOCKET(sockid) + W5100_SOCK_DPORT, destport);
}

uint8_t socket_tx_prepare(net_socket_t sockid)
{
    if (socket_tx_rd(sockid) != socket_tx_wr(sockid)) // Data is pending
        return 0;
    
    socket_tx_inc_wr(sockid, 0); // Fix sending empty packets

    return 1;
}

net_offset_t socket_tx_write(net_socket_t sockid, net_offset_t write_offset, net_size_t length, const uint8_t *data)
{
    while(length > socket_tx_fsr(sockid)) // Wait for free space
    {
    }

    net_offset_t offset = socket_tx_wr(sockid) + write_offset;
    net_addr_t start = SOCKET_TX_BASE_S(sockid)+ offset;

    if (start + length >= SOCKET_TX_END_S(sockid)) // If we need to split it up ..
    {
        net_size_t first_size = SOCKET_TX_END_S(sockid) - start;
        net_size_t second_size = length - first_size;

        w5100_write_array(start, first_size, data);
        w5100_write_array(SOCKET_TX_BASE_S(sockid), second_size, data + first_size);
    }
    else
    {
        w5100_write_array(start, length, data);
    }

    socket_tx_inc_wr(sockid, write_offset + length);

    return write_offset + length;
}

void socket_tx_flush(net_socket_t sockid)
{
    socket_cmd(sockid, SOCKET_CMD_SEND);

    // Wait for sending to be complete
    while (!(socket_ir(sockid) & W5100_SOCK_IR_SEND_OK))
    {
    }
    // Reset the SEND_OK interrupt
    socket_ir_clear(sockid, W5100_SOCK_IR_SEND_OK);
}


net_size_t socket_tx_fsr(net_socket_t sockid)
{
    return w5100_read16(W5100_SOCKET(sockid) + W5100_SOCK_TX_FSR);
}

net_offset_t socket_tx_wr(net_socket_t sockid)
{
    return w5100_read16(W5100_SOCKET(sockid) + W5100_SOCK_TX_WR) & SOCKET_TX_MASK_S(sockid);
}

void socket_tx_inc_wr(net_socket_t sockid, net_size_t inc)
{
    w5100_write16_add(W5100_SOCKET(sockid) + W5100_SOCK_TX_WR, inc);
}

net_offset_t socket_tx_rd(net_socket_t sockid)
{
    return w5100_read16(W5100_SOCKET(sockid) + W5100_SOCK_TX_RD) & SOCKET_TX_MASK_S(sockid);
}

net_size_t socket_rx_read(net_socket_t sockid, net_offset_t read_offset, net_size_t count, uint8_t *data)
{
    net_size_t available = socket_rx_rsr(sockid) - read_offset;
    if (available < 0)
        return -1;
    net_size_t length = MIN(available, count);

    net_offset_t offset = socket_rx_rd(sockid) + read_offset;
    net_addr_t start = SOCKET_RX_BASE_S(sockid) + offset;

    if (start + length >= SOCKET_RX_END_S(sockid)) // If we need to split it up ..
    {
        net_size_t first_size = SOCKET_RX_END_S(sockid) - start;
        net_size_t second_size = length - first_size;

        w5100_read_array(start, first_size, data);
        w5100_read_array(SOCKET_RX_BASE_S(sockid), second_size, data + first_size);
    }
    else
    {
        w5100_read_array(start, length, data);
    }

    return length;
}

net_size_t socket_rx_flush(net_socket_t sockid, net_size_t count)
{
    net_size_t available = socket_rx_rsr(sockid);
    net_size_t length = MIN(available, count);

    socket_rx_inc_rd(sockid, length);
    socket_cmd(sockid, SOCKET_CMD_RECV);

    return length;
}

net_size_t socket_rx_rsr(net_socket_t sockid)
{
    return w5100_read16(W5100_SOCKET(sockid) + W5100_SOCK_RX_RSR);
}

void socket_rx_inc_rd(net_socket_t sockid, net_size_t inc)
{
    w5100_write16_add(W5100_SOCKET(sockid) + W5100_SOCK_RX_RD, inc);
}

net_size_t socket_rx_rd(net_socket_t sockid)
{
    return w5100_read16(W5100_SOCKET(sockid) + W5100_SOCK_RX_RD) & SOCKET_RX_MASK_S(sockid);
}


