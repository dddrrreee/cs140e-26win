#include "rpi.h"

void notmain(void) {
    for(int i = 0; i < 32; i++)
        gpio_int_rising_edge(i);
}
