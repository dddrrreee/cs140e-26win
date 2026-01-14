#include "rpi.h"

// #define BAUD 115200
// #define CYCLES_PER_BIT 6076

// static inline void cycle_cnt_init(void) {
//     uint32_t in = 1;
//     asm volatile("MCR p15, 0, %0, c15, c12, 0" :: "r"(in));
// }

// static inline uint32_t cycle_cnt_read(void) {
//     uint32_t out;
//     asm volatile("MRC p15, 0, %0, c15, c12, 1" : "=r"(out));
//     return out;
// }

// /**
//  * Assumption: clock cycles to do these lines of code are negligible
//  *             compared to error correction of UART
//  * 
//  * Baud rate of 115200 --> 8.681s per bit
//  * 700e6 cycles/s --> 6076 cycles per bit
//  */
// void transmit_byte(unsigned pin, volatile uint8_t c) {
    
//     // Send start bit
//     gpio_set_off(pin); 
//     delay_cycles(CYCLES_PER_BIT);

//     // Send bits of the byte
//     for (uint8_t i = 7; i >= 0; i--) {

//         if ( (c >> i) & 1) // checks i-th bit
//             gpio_set_on(pin); 
//         else
//             gpio_pud_off(pin);
        
//         delay_cycles(CYCLES_PER_BIT);
//     }

//     // Send stop bit
//     gpio_set_on(pin); 
//     delay_cycles(CYCLES_PER_BIT);
// }


// #define UART_PERIOD 1000000/BAUD

void notmain(void) {
    // enum { tx = 24, led = 47 };
    enum { led = 47 };

    // Start cycle counter for timing measurements
    // cycle_cnt_init();

    // "Idle (logic high (1))" -wikipedia
    // gpio_set_output(tx);
    gpio_set_output(led);
    // gpio_set_on(tx); 
    // delay_cycles(1000000);

    // transmit
    while(1) {
        gpio_set_on(led);
        // transmit_byte(tx, 'H');
        // transmit_byte(tx, 'i');
        // transmit_byte(tx, '\n');
        // transmit_byte(tx, '\r');
        delay_cycles(1000000);
        gpio_set_off(led);
        delay_cycles(1000000);
    }
}
