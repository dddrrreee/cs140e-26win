// part 4

#include "rpi.h"

void notmain(void) {
    enum { led1 = 20, hat_led = 27, pi_led = 47 };

    gpio_set_output(led1);
    gpio_set_output(hat_led);
    gpio_set_output(pi_led);
    while(1) {
        gpio_set_on(led1);
        gpio_set_on(hat_led);
        gpio_set_off(pi_led);
        delay_cycles(1000000);
        gpio_set_off(led1);
        gpio_set_off(hat_led);
        gpio_set_on(pi_led);
        delay_cycles(1000000);
    }
}
