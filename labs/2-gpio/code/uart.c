/*
 * Functions used for the uart extension
 */
#include "rpi.h"

#define BAUD 115200
#define CYCLES_PER_BIT 6076

void uart_tx_byte(unsigned pin, volatile uint8_t c) {
    
    uint32_t cycle;

    // Send start bit
    cycle = cycle_cnt_read();
    gpio_set_off(pin); 
    
    
    while (cycle_cnt_read() - cycle < CYCLES_PER_BIT) {nop();}

    // Send bits of the byte
    for (volatile int8_t i = 0; i < 8; i++) {
        cycle = cycle_cnt_read();
        if ( (c >> i) & 0x1 ) // checks i-th bit
            gpio_set_on(pin); 
        else
            gpio_set_off(pin);
        while (cycle_cnt_read() - cycle < CYCLES_PER_BIT) {nop();}
    }

    // Send stop bit
    cycle = cycle_cnt_read();
    gpio_set_on(pin);
    while (cycle_cnt_read() - cycle < CYCLES_PER_BIT) {nop();}
}


void putk(unsigned pin, const char* str) {
    const char* s_ptr = str;

    // Transmits until the 0 byte
    while(*s_ptr) {
        uart_tx_byte(pin, *s_ptr);
        s_ptr += 1;
    }
}