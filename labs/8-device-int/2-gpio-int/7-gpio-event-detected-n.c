#include "rpi.h"

void notmain(void) {
    for(int i = 0; i < 32; i++)
        trace("event detected = %d\n", gpio_event_detected(i));
}
