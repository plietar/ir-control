#include "timer.h"
#include <avr/interrupt.h>
#include <avr/io.h>
static uint16_t timer_count = 0;
/*
ISR(TIMER1_OVF_vect) {
    TCNT1=0x0BDC; // set initial value to remove time error (16bit counter register)
    timer_count ++;  
}
*/
void timer_init()
{
    timer_count = 0;
    /*
    TIMSK1=0x01; // enabled global and timer overflow interrupt;
    TCCR1A = 0x00; // normal operation page 148 (mode0);
    TCNT1=0x0BDC; // set initial value to remove time error (16bit counter register)
    TCCR1B = 0x04; // start timer/ set clock
    */
}

uint16_t timer_secs()
{
    return timer_count;
}

