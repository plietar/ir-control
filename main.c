#include "spi.h"
#include "w5100.h"
#include "uart.h"
#include "socket.h"
#include "udp.h"
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
    printf_P(PSTR("CRC32 \"HELLO\" : %8lX\n\r"), crc32_bytes((const uint8_t*)"HELLO", 5));
    printf_P(PSTR("CRC16 \"HELLO\" : %4hX\n\r"), crc16_bytes((const uint8_t*)"HELLO", 5));

    uint16_t *buffer = NULL;

    net_socket_t sockid = SOCKET_INVALID;
    uint16_t port = 5000;

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

    sockid = udp_open(port);
    printf_P(PSTR("Socket %u opened on port %d\n\r"), sockid, port);
    
    while(1) {
        net_size_t available = -1;
        if ((available = udp_rx_available(sockid)) > 0)
        {
            ir_stop();
            free(buffer); // Free the previous buffer. TODO stop the ir before.
            buffer = malloc(available);
            if (buffer)
            {
                udp_rx_read(sockid, 0, available, (uint8_t *)buffer);
                
                for (int i = 0; i < (available >> 1); i++)
                {
                    buffer[i] = util_ntohs(buffer[i]);
                }

                if (available >= 8 && (uint16_t)available >= 2*(4 + 2 * (buffer[2] + buffer[3])))
                {
                    uint32_t crc = crc32_bytes((uint8_t*)buffer, available);
                    printf_P(PSTR("NEW sequence avail: %d, %d single, %d repeat, crc: %8lX \n\r"), available, buffer[2], buffer[3], crc);

                    for (int i = 0; i < available; i++)
                    {
                        printf("%02X ", ((uint8_t*)buffer)[i]);
                        if (i % 2)
                            printf("\n\r");
                    }
                    printf("\n\r");

                    ir_start(buffer);
                }
                else
                {
                    printf("Bad seq: %d available, %d single, %d repeat\n\r", available, buffer[2], buffer[3]);
                }
            }
            else
            {
                printf("Malloc failed\n\r");
            }

            udp_rx_flush(sockid);
        }

    }
    return 0;
}

int uart_putc_printf(char c, FILE *f __attribute__((unused)))
{
    uart_putc(c);
    return 0;
}
