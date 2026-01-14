// part 7

#include "rpi.h"

void notmain(void) {
    enum { tx = 14, led = 47, pulldown = 9, pullup = 8};

    cycle_cnt_init();

    // Enable bb uart
    gpio_set_output(tx);
    gpio_set_on(tx);

    gpio_set_input(pulldown);
    gpio_set_pulldown(pulldown);
    gpio_set_input(pullup);
    gpio_set_pullup(pullup);

    delay_cycles(1000000);

    while(1) {
        putk(tx, "Pulldown pin (9) reads: ");
        uart_tx_byte(tx, gpio_read(pulldown) + '0'); // Turns 0 and 1 into '0' and '1' characters
        putk(tx, "\n");

        putk(tx, "Pullup pin (8) reads: ");
        uart_tx_byte(tx, gpio_read(pullup) + '0'); // Turns 0 and 1 into '0' and '1' characters
        putk(tx, "\n");
        putk(tx, "\n");
        
        delay_cycles(1000000);
    }
}
