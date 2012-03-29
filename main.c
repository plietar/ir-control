#include "uart.h"
#include "w5100.h"
#include "udp.h"
#include "dhcp.h"
#include "timer.h"
#include "ir.h"
#include "util.h"
#include "tcp.h"

#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
uint8_t ip[] = {0, 0, 0, 0 };
uint8_t subnet[] = {0, 0, 0, 0};
uint8_t gw[] = {0, 0, 0, 0 };

int debug_putc_printf(char c, FILE *f);

int main(void)
{
    net_socket_t ir_server = SOCKET_INVALID;
    net_socket_t debug_server = SOCKET_INVALID;
    net_socket_t debug_client = SOCKET_INVALID;
    
    uint16_t ir_port = 5000;
    uint16_t debug_port = 5000;

    uint16_t *ir_buffer = NULL;
    uint8_t ret = 0;
    net_size_t available = -1;
    
    FILE debug_stdout = FDEV_SETUP_STREAM(debug_putc_printf, NULL, _FDEV_SETUP_WRITE);
    debug_stdout.udata = &debug_client;

    timer_init();
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    stderr = stdout = &debug_stdout;

    sei();
    _delay_ms(100);
    printf_P(PSTR("Hello from printf\n\r"));

    w5100_init();
    
    printf_P(PSTR("W5100 Init done ..\n\r"));
    
    w5100_set_hwaddr(mac);
    w5100_set_ipaddr(ip);
    w5100_set_subnet(subnet);
    w5100_set_gateway(gw);

    _delay_ms(5000);
    
    printf_P(PSTR("W5100 Ready ..\n\r"));
    
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
    printf_P(PSTR("DHCP OK, ip: %u.%u.%u.%u\n\r"),ip[0],ip[1],ip[2],ip[3]);

    ir_server = udp_open(ir_port);
    printf_P(PSTR("IR Server opened on port %u, socket %u\n\r"), ir_port, ir_server);
    
    while(1) {
        
        if (debug_client == SOCKET_INVALID && debug_server == SOCKET_INVALID) // No one is connected to the debug port
        {
            debug_server = tcp_open(debug_port);
            if (debug_server != SOCKET_INVALID)
            {
                printf_P(PSTR("Debug server opened on port %u, socket %u\n\r"), debug_port, debug_server);
                tcp_listen(debug_server);
            }
        }
        else if (debug_client == SOCKET_INVALID)
        {
            debug_client = tcp_accept(&debug_server);
            if (debug_client != SOCKET_INVALID) // Someone has connected
            {
                printf_P(PSTR("Welcome to the Debug Connection\n\rYour ip is u.u.u.u\n\rYour are connected to socket %u\n\r"), debug_client);
                if (debug_server != SOCKET_INVALID)
                {
                    printf_P(PSTR("Killing the debug server\n\r"));
                    tcp_close(debug_server);
                    debug_server = SOCKET_INVALID;
                }
            }
        }
        else if (!tcp_connected(debug_client)) // Have we lost the debug connection
        {
            printf_P(PSTR("Debug client has disconnected\n\r"));
            tcp_close(debug_client);
            debug_client = SOCKET_INVALID;
        }

        if ((available = udp_rx_available(ir_server)) > 0)
        {
            //ir_stop();
            free(ir_buffer); // Free the previous buffer. TODO stop the ir before.
            ir_buffer = malloc(available);
            if(ir_buffer)
            {
                udp_rx_read(ir_server, 0, available, (uint8_t *)ir_buffer);
                
                for (int i = 0; i < (available >> 1); i++)
                {
                    ir_buffer[i] = util_ntohs(ir_buffer[i]);
                }

                if (available >= 8 && (uint16_t)available >= 2*(4 + 2 * (ir_buffer[2] + ir_buffer[3])))
                {
                    printf_P(PSTR("New valid IR code available: %d, %d single, %d repeat\n\r"), available, ir_buffer[2], ir_buffer[3]);

                    ir_start(ir_buffer);
                }
                else
                {
                    printf("Bad ir code: %d available, %d single, %d repeat\n\r", available, ir_buffer[2], ir_buffer[3]);
                }
            }
            else
            {
                printf("Malloc failed\n\r");
            }

            udp_rx_flush(ir_server);
        }

    }
    return 0;
}

int debug_putc_printf(char c, FILE *f)
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
