// simple mini-uart driver: implement every routine 
// with a <todo>.
//
// NOTE: 
//  - from broadcom: if you are writing to different 
//    devices you MUST use a dev_barrier().   
//  - its not always clear when X and Y are different
//    devices.
//  - pay attenton for errata!   there are some serious
//    ones here.  if you have a week free you'd learn 
//    alot figuring out what these are (esp hard given
//    the lack of printing) but you'd learn alot, and
//    definitely have new-found respect to the pioneers
//    that worked out the bcm eratta.
//
// historically a problem with writing UART code for
// this class (and for human history) is that when 
// things go wrong you can't print since doing so uses
// uart.  thus, debugging is very old school circa
// 1950s, which modern brains arne't built for out of
// the box.   you have two options:
//  1. think hard.  we recommend this.
//  2. use the included bit-banging sw uart routine
//     to print.   this makes things much easier.
//     but if you do make sure you delete it at the 
//     end, otherwise your GPIO will be in a bad state.
//
// in either case, in the next part of the lab you'll
// implement bit-banged UART yourself.
#include "rpi.h"

#include "rpi-thread.h" // For thread

// change "1" to "0" if you want to comment out
// the entire block.
#if 1
//*****************************************************
// We provide a bit-banged version of UART for debugging
// your UART code.  delete when done!
//
// NOTE: if you call <emergency_printk>, it takes 
// over the UART GPIO pins (14,15). Thus, your UART 
// GPIO initialization will get destroyed.  Do not 
// forget!   

// header in <libpi/include/sw-uart.h>
#include "sw-uart.h"
static sw_uart_t sw_uart;

// a sw-uart putc implementation.
static int sw_uart_putc(int chr) {
    sw_uart_put8(&sw_uart,chr);
    return chr;
}

// call this routine to print stuff. 
//
// note the function pointer hack: after you call it 
// once can call the regular printk etc.
__attribute__((noreturn)) 
static void emergency_printk(const char *fmt, ...)  {
    // we forcibly initialize in case the 
    // GPIO got reset. this will setup 
    // gpio 14,15 for sw-uart.
    sw_uart = sw_uart_default();

    // all libpi output is via a <putc>
    // function pointer: this installs ours
    // instead of the default
    rpi_putchar_set(sw_uart_putc);

    // do print
    va_list args;
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);

    // at this point UART is all messed up b/c we took it over
    // so just reboot.   we've set the putchar so this will work
    clean_reboot();
}

#undef todo
#define todo(msg) do {                          \
    emergency_printk("%s:%d:%s\nDONE!!!\n",     \
            __FUNCTION__,__LINE__,msg);         \
} while(0)

// END of the bit bang code.
#endif
//*****************************************************
// ** REGISTERS
//*****************************************************

enum {
    AUX_IRQ = 0x20215000,
    AUX_ENABLES  = (AUX_IRQ + 0x04),
    AUX_MU_IO  = (AUX_IRQ + 0x40),
    AUX_MU_IER  = (AUX_IRQ + 0x44),
    AUX_MU_IIR  = (AUX_IRQ + 0x48),
    AUX_MU_LCR  = (AUX_IRQ + 0x4C),
    AUX_MU_MCR  = (AUX_IRQ + 0x50),
    AUX_MU_LSR  = (AUX_IRQ + 0x54),
    AUX_MU_MSR  = (AUX_IRQ + 0x58),
    AUX_MU_SCRATCH  = (AUX_IRQ + 0x5C),
    AUX_MU_CNTL  = (AUX_IRQ + 0x60),
    AUX_MU_STAT  = (AUX_IRQ + 0x64),
    AUX_MU_BAUD  = (AUX_IRQ + 0x68),
};

//*****************************************************
// the rest you should implement.

// called first to setup uart to 8n1 115200  baud,
// no interrupts.
//  - you will need memory barriers, use <dev_barrier()>
//
//  later: should add an init that takes a baud rate.
void uart_init() {

    // UHH assumes that GPIO is initted
    dev_barrier();

    // BRUH I HAD TO INITIALZIE THIS AS ALT5
    gpio_set_function(GPIO_TX, GPIO_FUNC_ALT5);
    gpio_set_function(GPIO_RX, GPIO_FUNC_ALT5);

    volatile uint32_t val;

    // Enable UART
    dev_barrier();
    PUT32(AUX_ENABLES, GET32(AUX_ENABLES) | 1); 
    dev_barrier();
    
    // Disable UART RX/TX
    PUT32(AUX_MU_CNTL, 0); 

    // Disable Interrupts
    PUT32(AUX_MU_IER, 0); 
    
    // 8N1 (already N1 by default)
    PUT32(AUX_MU_LCR, 0b11); 
    
    // Set baud rate     115200 = 250MHz / (8 * (270 + 1))
    PUT32(AUX_MU_BAUD, 270); 
    
    // Clear FIFO
    PUT32(AUX_MU_IIR, 0b110); 

    // Smashing AUX_MU_MCR reg
    PUT32(AUX_MU_MCR, 0);

    // Enable RX/TX
    PUT32(AUX_MU_CNTL, 0b11); 
    
    dev_barrier();
    
}

// disable the uart: make sure all bytes have been
// 
void uart_disable(void) {
    dev_barrier();

    // Flush TX FIFO
    uart_flush_tx();

    // Disable UART RX/TX
    PUT32(AUX_MU_CNTL, GET32(AUX_MU_CNTL) & ~0b11); 

    dev_barrier();

    // Disable UART Peripheral (no more access to the regs boohoo) 
    PUT32(AUX_ENABLES, GET32(AUX_ENABLES) & ~1); 

    dev_barrier();
}

// returns one byte from the RX (input) hardware
// FIFO.  if FIFO is empty, blocks until there is 
// at least one byte.
int uart_get8(void) {
    dev_barrier();
    while (!uart_has_data()) { rpi_yield(); } // BLOCKS until there is data :(
    uint32_t val = GET32(AUX_MU_IO);
    dev_barrier();
    return val & 0xFF;
}

// returns 1 if the hardware TX (output) FIFO has room
// for at least one byte.  returns 0 otherwise.
int uart_can_put8(void) {
    uint32_t val = GET32(AUX_MU_LSR);
    return val & 1<<5;
}

// put one byte on the TX FIFO, if necessary, waits
// until the FIFO has space.
int uart_put8(uint8_t c) {
    dev_barrier();
    while (!(uart_can_put8())) { rpi_yield(); } // BLOCKS until can send data :(
    PUT32(AUX_MU_IO, c);
    dev_barrier();
    return 1;
}

// returns:
//  - 1 if at least one byte on the hardware RX FIFO.
//  - 0 otherwise
int uart_has_data(void) { // ** IMPLEMENTED
    uint32_t val = GET32(AUX_MU_LSR);
    return val & 1;
}

// returns:
//  -1 if no data on the RX FIFO.
//  otherwise reads a byte and returns it.
int uart_get8_async(void) { 
    if(!uart_has_data())
        return -1;
    return uart_get8();
}

// returns:
//  - 1 if TX FIFO empty AND idle.
//  - 0 if not empty.
int uart_tx_is_empty(void) {
    // TODO: implement this!
    uint32_t val = GET32(AUX_MU_LSR);
    return val & 1<<6;
}

// return only when the TX FIFO is empty AND the
// TX transmitter is idle.  
//
// used when rebooting or turning off the UART to
// make sure that any output has been completely 
// transmitted.  otherwise can get truncated 
// if reboot happens before all bytes have been
// received.
void uart_flush_tx(void) {
    while(!uart_tx_is_empty())
        rpi_yield(); 
}
