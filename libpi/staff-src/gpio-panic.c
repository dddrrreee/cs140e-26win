#include "rpi.h"

// annoying that we need to handle varargs.  smh.
void gpio_panic(const char *fmt, ...) {
    panic("GPIO_PANIC: fixme\n");
}
