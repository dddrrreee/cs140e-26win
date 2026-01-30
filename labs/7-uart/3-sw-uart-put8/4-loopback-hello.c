// use the software uart to send to the hardware uart using a 
// loopback jumper.
#include "rpi.h"
#include "sw-uart.h"

void notmain(void) {
    trace("put a jumper from pin 19 to pin 15 (rx)\n");

    // use pin 19 for tx, [ignore: 20=rx]
    sw_uart_t u = sw_uart_init(19,20, 115200);

    const char *test = "hello this is us doing loopback\n";
    for(const char *p = test; *p; p++)  {
        sw_uart_put8(&u, *p);

        let c = uart_get8();
        if(c != *p)
            panic("sent %c, got %c\n", *p, c);
        else
            trace("yea!: sent %c, got %c\n", *p, c);
    }

    trace("all done!\n");
}
