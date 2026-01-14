// part 4

#include "rpi.h"

void notmain(void) {
    enum { led = 47 };

    gpio_set_output(led);
    while(1) {
        gpio_set_on(led);
        delay_cycles(1000000);
        gpio_set_off(led);
        delay_cycles(1000000);
    }
}
