#include "spi.h"
#include "w5100.h"
#include "uart.h"
#include "socket.h"
#include "udp.h"
#include "dhcp.h"
#include "timer.h"

#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#define BUFFER_SIZE 3 // Very small buffer

uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
uint8_t ip[] = {0, 0, 0, 0 };
uint8_t subnet[] = {0, 0, 0, 0};
uint8_t gw[] = {0, 0, 0, 0 };

int uart_putc_printf(char c, FILE *f);

static FILE serial_stdout = FDEV_SETUP_STREAM(uart_putc_printf, NULL, _FDEV_SETUP_WRITE); 

int main(void)
{
    net_socket_t sockid = SOCKET_INVALID;
    uint8_t buffer[BUFFER_SIZE];
    net_size_t length = 0;
    net_offset_t offset = 0;
    uint16_t port = 5000;

    timer_init();
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    stderr = stdout = &serial_stdout;

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
    printf_P(PSTR("Got ip address: %d.%d.%d.%d \n\r"), ip[0], ip[1], ip[2], ip[3]);
    printf_P(PSTR("Got gw address: %d.%d.%d.%d \n\r"), gw[0], gw[1], gw[2], gw[3]);
    printf_P(PSTR("Got sn address: %d.%d.%d.%d \n\r"), subnet[0], subnet[1], subnet[2], subnet[3]);

    sockid = udp_open(port);
    printf_P(PSTR("Socket %u opened on port %d\n\r"), sockid, port);
    
    while(1) {
        if (udp_rx_available(sockid) > 0)
        {
            struct udp_w5100_header header;
            udp_rx_header(sockid, &header);

            printf_P(PSTR("Received a new packet from %d\n\r"), header.destip[0]);
            udp_tx_prepare(sockid, header.destip, header.destport);
            while ((length = udp_rx_read(sockid, offset, BUFFER_SIZE, buffer)) > 0)
            {
                udp_tx_write(sockid, offset, length, buffer);
                for (int i = 0; i < length; i++)
                {
                    putchar(buffer[i]);
                }
                
                offset += length;
            }
            offset = 0;
            udp_tx_flush(sockid);
            udp_rx_flush(sockid);
            printf("\n\r");
        }

        _delay_ms(1000);
    }
    
    return 0;
}

int uart_putc_printf(char c, FILE *f __attribute__((unused)))
{
    uart_putc(c);
    return 0;
}
