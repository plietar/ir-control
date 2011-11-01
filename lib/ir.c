#include "ir.h"
#include <avr/interrupt.h>

static const uint16_t *ir_code;
static uint16_t ir_cycle_count;
static uint16_t ir_total_cycle_count;
static uint8_t ir_seq_index;

static uint8_t ir_led_state;

void ir_init()
{
}

void ir_on()
{
    IR_TCCRnA |= (1<<IR_COMn0) | (1<<IR_COMn1);
    ir_led_state = 1;
}

void ir_off()
{
    IR_TCCRnA &= ~((1<<IR_COMn0) | (1<<IR_COMn1));
    ir_led_state = 0;
}

void ir_toggle()
{
    if (ir_led_state)
        ir_off();
    else
        ir_on();
}

void ir_start(uint16_t *code)
{
    ir_code = code;
    IR_PORT &= ~IR_BV; // Turn output off
    IR_DDR |= IR_BV; // Set it as output

    IR_TCCRnA = 0x00; // Reset the pwm
    IR_TCCRnB = 0x00;

    uint16_t top = ( (F_CPU/1000000.0) * code[PRONTO_FREQ_CODE] * 0.241246 ) - 1;
    IR_ICRn = top;
    IR_OCRn = top >> 1;

    IR_TCCRnA = (1<<WGM11);
    IR_TCCRnB = (1<<WGM13) | (1<<WGM12);

    IR_TCNTn = 0x00;
    IR_TIFRn = 0x00;

    IR_TIMSKn = 1 << IR_TOIEn;

    ir_on();
    IR_TCCRnB |= (1<<CS10);
}

ISR(TIMER1_OVF_vect) {
    uint16_t sequenceIndexEnd;          // Index to the last element in the 
                                        // ProntoCode array
    uint16_t repeatSequenceIndexStart;  // Index to ProntoCode array to repeat

    ir_total_cycle_count++;
    ir_cycle_count++;
    
    // End of this state, toggle led and move to the next state
    if (ir_cycle_count== ir_code[ir_seq_index]) {
        ir_toggle();
        ir_cycle_count = 0;
        ir_seq_index++;
        sequenceIndexEnd = PRONTO_CODE_START +
                           (ir_code[PRONTO_SEQUENCE1_LENGTH]<<1) +
                           (ir_code[PRONTO_SEQUENCE2_LENGTH]<<1);
                                    
        repeatSequenceIndexStart = PRONTO_CODE_START +
                                   (ir_code[PRONTO_SEQUENCE1_LENGTH]<<1);

        // If index past last element in array, set index to repeat
        if (ir_seq_index >= sequenceIndexEnd ) {        
            ir_seq_index = repeatSequenceIndexStart;     

/*            if(ir_total_cycle_count>TOTAL_CYCLES) {            // Finished
                TCCR1B &= ~(1<<CS10);                    // Stop Timer
                IRLED_Power(false);                        // Turn off LED
            }*/
        }
    }
}
