// engler, cs140 put your gpio-int implementations in here.
#include "rpi.h"

// in libpi/include: has useful enums.
#include "rpi-interrupts.h"
#include "rpi-regs.h"

// returns 1 if there is currently a GPIO_INT0 interrupt, 
// 0 otherwise.
//
// note: we can only get interrupts for <GPIO_INT0> since the
// (the other pins are inaccessible for external devices).
int gpio_has_interrupt(void) { // ** 
    // todo("implement: is there a GPIO_INT0 interrupt?\n");
    dev_barrier();

    // Read interrupt register IRQ PENDING 2
    volatile unsigned value = GET32(IRQ_PENDING_2);
    dev_barrier();
    return (value >> (49-32)) & 1;

}

// static void OR32(uint32_t addr, uint32_t x) {
//     device_barrier();
//     PUT32(addr, get32(addr) | val);
//     device_barrier();
// }

// p97 set to detect rising edge (0->1) on <pin>.
// as the broadcom doc states, it  detects by sampling based on the clock.
// it looks for "011" (low, hi, hi) to suppress noise.  i.e., its triggered only
// *after* a 1 reading has been sampled twice, so there will be delay.
// if you want lower latency, you should us async rising edge (p99)
//
// also have to enable GPIO interrupts at all in <IRQ_Enable_2>
void gpio_int_rising_edge(unsigned pin) {
    if(pin>=32)
        return;

    dev_barrier();

    // Enable rising edge
    OR32(GPREN0, 1 << pin); // Only GPREN0 (not GPREN0 too) because pin is less than 32
    
    dev_barrier();
    
    // Enable interrupt
    PUT32(INT_EN_2, 1 << (49 - 32));

    dev_barrier();
}

// p98: detect falling edge (1->0).  sampled using the system clock.  
// similarly to rising edge detection, it suppresses noise by looking for
// "100" --- i.e., is triggered after two readings of "0" and so the 
// interrupt is delayed two clock cycles.   if you want  lower latency,
// you should use async falling edge. (p99)
//
// also have to enable GPIO interrupts at all in <IRQ_Enable_2>
void gpio_int_falling_edge(unsigned pin) { // ** 
    if(pin>=32)
        return;

    dev_barrier();
    
    // Enable rising edge
    OR32(GPFEN0, 1 << pin); // Only GPFEN0 (not GPFEN1 too) because pin is less than 32
    
    dev_barrier();
    
    // Enable interrupt
    PUT32(INT_EN_2, 1 << (49 - 32));

    dev_barrier();
}

// p96: a 1<<pin is set in EVENT_DETECT if <pin> triggered an interrupt.
// if you configure multiple events to lead to interrupts, you will have to 
// read the pin to determine which caused it.
int gpio_event_detected(unsigned pin) { // **
    if(pin >= 32)  {
        // gpio_panic("illegal pin=%d\n", pin);
        return 0;
    }

    dev_barrier();

    volatile int detected = 0;
    if (GET32(GPEDS0) & (1 << pin)) {
        detected = 1;
    }

    dev_barrier();

    return detected;
}

// p96: have to write a 1 to the pin to clear the event.
void gpio_event_clear(unsigned pin) { // **
    if(pin>=32)
        return;
    // todo("implement: clear event on <pin>\n");
    dev_barrier();
    PUT32(GPEDS0, 1 << pin);
    dev_barrier();
}