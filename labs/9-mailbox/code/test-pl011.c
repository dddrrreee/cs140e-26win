#include "rpi.h"
#include "pl011-uart.h"

void notmain(void) {
    output("hello from miniUART\n");
    uart_flush_tx();

    pl011_console_init();
    output("hello from pl011\n");

}
