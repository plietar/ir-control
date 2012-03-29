#include "spi.h"
#include "w5100.h"
#include "uart.h"
#include "socket.h"
#include "tcp.h"
#include "dhcp.h"
#include "timer.h"
#include "ir.h"
#include "util.h"
#include "crc.h"

#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>

uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

int uart_putc_printf(char c, FILE *f);
int tcp_putc_printf(char c, FILE *f);


int main(void)
{
    uint8_t ip[] = {0, 0, 0, 0 };
    uint8_t subnet[] = {0, 0, 0, 0};
    uint8_t gw[] = {0, 0, 0, 0 };
    
    net_socket_t server = SOCKET_INVALID;
    net_socket_t client = SOCKET_INVALID;
    uint16_t port = 5000;

//    FILE serial_stdout = FDEV_SETUP_STREAM(uart_putc_printf, NULL, _FDEV_SETUP_WRITE); 
    FILE tcp_stdout = FDEV_SETUP_STREAM(tcp_putc_printf, NULL, _FDEV_SETUP_WRITE); 
    tcp_stdout.udata = &client;

    net_size_t count = 0;
    
    timer_init();
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    stderr = stdout = &tcp_stdout;

    sei();
    _delay_ms(100);
    printf_P(PSTR("Serial hello from printf\n\r"));

    
    w5100_init();
    printf_P(PSTR("W5100 Init done ..\n\r"));
    
    w5100_set_hwaddr(mac);
    w5100_set_ipaddr(ip);
    w5100_set_subnet(subnet);
    w5100_set_gateway(gw);

    _delay_ms(5000);
    
    printf_P(PSTR("Ready ..\n\r"));
     
    uint8_t ret = 0;
    do
    {
        ret = dhcp_get_ip();

        if(ret != 0)
        {
            printf("DHCP failed! Ret: %d\n\r", ret);
            _delay_ms(1000);
        }
    }
    while(ret != 0);

    w5100_get_ipaddr(ip);
    w5100_get_gateway(gw);
    w5100_get_subnet(subnet);
    printf_P(PSTR("DHCP OK\r\n"));
    printf_P(PSTR("ip: %d.%d.%d.%d\n\r"),ip[0],ip[1],ip[2],ip[3]);

    while(1) {
        if (client == SOCKET_INVALID && server == SOCKET_INVALID) // Nobody is connected and we aren't accepting new connections
        {
            server = tcp_open(port);
            tcp_listen(server);
            printf_P(PSTR("Socket %u opened on port %d\n\r"), server, port);
        }
        else if (client == SOCKET_INVALID)
        {
            client = tcp_accept(&server);
            if (client != SOCKET_INVALID)
            {
                printf_P(PSTR("New client : %d\n\r"), client);
                if (server != SOCKET_INVALID)
                {
                    printf_P(PSTR("Killing server\n\r"));
                    tcp_close(server); // Don't let others connect
                    server = SOCKET_INVALID;
                }
            }
        }
        else if(!tcp_connected(client))
        {
            tcp_disconnect(client);
            client = SOCKET_INVALID;
            printf_P(PSTR("Lost connection with client\n\r"));
        }
        else if ((count = tcp_rx_available(client)) > 0)
        {
            uint8_t *data = malloc(count + 1); // Add null character
            if (data)
            {
                data[count] = 0;

                tcp_rx_read(client, 0, count, data);
                net_size_t size = count;
                while (data[size-1] == '\n' || data[size-1] == '\r') // Removing trailing new line chararcters
                {
                    data[size-1] = 0;
                    size--;
                }

                printf_P(PSTR("You said \"%s\"\n\r"), data);

                free(data);
            }
            else
                printf_P(PSTR("MALLOC Failed\n\r"));

            tcp_rx_flush(client, count);
        }
    }
    return 0;
}

int tcp_putc_printf(char c, FILE *f)
{
    if (f->udata)
    {
        net_socket_t sockid = *(net_socket_t*)f->udata;

        if (sockid != SOCKET_INVALID && tcp_connected(sockid) && tcp_tx_prepare(sockid))
        {
            tcp_tx_write(sockid, 0, 1, (uint8_t*)&c);
            tcp_tx_flush(sockid);
        }
    }
    
    uart_putc(c);
    
    return 0;
}

int uart_putc_printf(char c, FILE *f __attribute__((unused)))
{
    uart_putc(c);
    return 0;
}
