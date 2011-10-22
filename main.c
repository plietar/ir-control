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

#define BUFFER_SIZE 10

uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
uint8_t ip[] = {0, 0, 0, 0 };
uint8_t subnet[] = {0, 0, 0, 0};
uint8_t gw[] = {0, 0, 0, 0 };

int uart_putc_printf(char c, FILE *f);

static FILE mystdout = FDEV_SETUP_STREAM(uart_putc_printf, NULL, _FDEV_SETUP_WRITE); 

int main(void)
{
    uint8_t sockid = SOCKET_INVALID;
    char buffer[BUFFER_SIZE];
    uint16_t length = 0;
    uint8_t destip[] = {192,168,0,119};
    uint16_t port = 5000;
    uint16_t count = 0;

    timer_init();
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    stderr = stdout = &mystdout;

    sei();

    _delay_ms(100);
    printf_P(PSTR("Hello from printf\n\r"));
    w5100_init();
    printf_P(PSTR("W5100 Init done ..\n\r"));
    
    w5100_set_hwaddr(mac);
    w5100_set_ipaddr(ip);
    w5100_set_subnet(subnet);
    w5100_set_gateway(gw);

    _delay_ms(100);
    printf_P(PSTR("Ready ..\n\r"));
    
    uint8_t ret = dhcp_get_ip(ip);

    if (ret)
    {
        w5100_get_ipaddr(ip);
        w5100_get_gateway(gw);
        w5100_get_subnet(subnet);
        printf("Got ip address: %d.%d.%d.%d \n\r", ip[0], ip[1], ip[2], ip[3]);
        printf("Got gw address: %d.%d.%d.%d \n\r", gw[0], gw[1], gw[2], gw[3]);
        printf("Got sn address: %d.%d.%d.%d \n\r", subnet[0], subnet[1], subnet[2], subnet[3]);
    }
    else
    {
        printf("DHCP did not work :(\n\r");
    }

/*
    sockid = udp_open(port);

    printf_P(PSTR("Got socket %d, ready: %d \n\r"), sockid, udp_ready(sockid));

    while(1) {
        count ++;
        length = snprintf_P(buffer, BUFFER_SIZE, PSTR("Test packet number %d\n\r"), count);
        udp_send(sockid, destip, port, length, (uint8_t *)buffer);

        if (udp_available(sockid) > 0)
        {
            length = udp_recv(sockid, BUFFER_SIZE - 1, buffer); // -1 to keep space for final NULL
            buffer[length] = 0; // Add final NULL
            printf_P(PSTR("Received: %s\n\r"), buffer);
        }

        _delay_ms(1000);
    }
    */
    while (1)
    {
        printf("Time: %d\n\r", timer_secs());
        _delay_ms(100);
    }
    return 0;
}

int uart_putc_printf(char c, FILE *f __attribute__((unused)))
{
    uart_putc(c);
    return 0;
}
