#ifndef _ir_h__
#define _ir_h__
#include <stdint.h>
#include <avr/io.h>

#define IR_PORT PORTB
#define IR_PIN PINB
#define IR_DDR DDRB
#define IR_BV _BV(1)

#define IR_OCR OCR1A
#define IR_TCCRnA TCCR1A
#define IR_TCCRnB TCCR1B
#define IR_TCNTn TCNT1
#define IR_TIFRn TIFR1
#define IR_TIMSKn TIMSK1
#define IR_TOIEn TOIE1
#define IR_ICRn ICR1
#define IR_OCRn OCR1A
#define IR_COMn0 COM1A0
#define IR_COMn1 COM1A1

#define PRONTO_IR_SOURCE            0   // Pronto code byte 0
#define PRONTO_FREQ_CODE            1   // Pronto code byte 1
#define PRONTO_SEQUENCE1_LENGTH     2   // Pronto code byte 2
#define PRONTO_SEQUENCE2_LENGTH     3   // Pronto code byte 3
#define PRONTO_CODE_START           4   // Pronto code byte 4

void ir_init();
void ir_on();
void ir_off();
void ir_toggle();
void ir_start(uint16_t *pronto);
void ir_stop();

#endif
