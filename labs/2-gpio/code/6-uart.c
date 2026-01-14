#include "rpi.h"


void notmain(void) {
    enum { tx = 14, led = 47 };
    // enum { led = 47 };

    // Start cycle counter for timing measurements
    cycle_cnt_init();

    // "Idle (logic high (1))" -wikipedia
    gpio_set_output(tx);
    gpio_set_output(led);
    gpio_set_on(tx); 
    delay_cycles(1000000);

    // transmit
    while(1) {
        gpio_set_on(led);

        putk(tx, "Hello world!\n");

        delay_cycles(1000000);
        gpio_set_off(led);
        delay_cycles(1000000);
    }
}
