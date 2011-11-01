#include "spi.h"
#include "w5100.h"
#include "uart.h"
#include "socket.h"
#include "udp.h"
#include "dhcp.h"
#include "timer.h"
#include "ir.h"

#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#define BUFFER_SIZE 200

uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
/*uint8_t ip[] = {192, 168, 0, 145 };
uint8_t subnet[] = {255, 255, 255, 0};
uint8_t gw[] = {0, 0, 0, 0 };
*/

uint8_t ip[] = {0, 0, 0, 0 };
uint8_t subnet[] = {0, 0, 0, 0};
uint8_t gw[] = {0, 0, 0, 0 };

int uart_putc_printf(char c, FILE *f);

static FILE serial_stdout = FDEV_SETUP_STREAM(uart_putc_printf, NULL, _FDEV_SETUP_WRITE); 

int main(void)
{
    timer_init();
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    stderr = stdout = &serial_stdout;

    sei();
    _delay_ms(100);
    printf_P(PSTR("Hello from printf\n\r"));

    uint16_t buffer[BUFFER_SIZE];
    net_socket_t sockid = SOCKET_INVALID;
    uint16_t port = 5000;

    w5100_init();
    printf_P(PSTR("W5100 Init done ..\n\r"));
    
    w5100_set_hwaddr(mac);
    w5100_set_ipaddr(ip);
    w5100_set_subnet(subnet);
    w5100_set_gateway(gw);

    _delay_ms(1000);
    printf_P(PSTR("Ready ..\n\r"));
    
    
    uint8_t ret = 0;
    do
    {
        ret = dhcp_get_ip();

        if(!ret)
        {
            printf_P(PSTR("DHCP failed!\n\r"));
            _delay_ms(1000);
        }
    }
    while(!ret);


    w5100_get_ipaddr(ip);
    w5100_get_gateway(gw);
    w5100_get_subnet(subnet);
    printf_P(PSTR("DHCP OK\n\r"));

    sockid = udp_open(port);
    printf_P(PSTR("Socket %u opened on port %d\n\r"), sockid, port);
    
    while(1) {
        net_size_t available = -1;
        if ((available = udp_rx_available(sockid)) > 0)
        {
            if (available <= BUFFER_SIZE)
            {
                udp_rx_read(sockid, 0, BUFFER_SIZE, (uint8_t *)buffer);
                ir_start(buffer);
                printf_P(PSTR("NEW sequence avail: %d\n\r"), available);
            }
            else
                printf_P(PSTR("Wrong sequence: avil: %d Buf: %d\n\r"), available, BUFFER_SIZE);

            udp_rx_flush(sockid);
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
