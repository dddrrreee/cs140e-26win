// engler, cs140 put your gpio-int implementations in here.
#include "rpi.h"

// in libpi/include: has useful enums.
#include "rpi-interrupts.h"

enum {
    GPIO_BASE = 0x20200000,
    // gpio_set0  = (GPIO_BASE + 0x1C),
    // gpio_clr0  = (GPIO_BASE + 0x28),
    // gpio_lev0  = (GPIO_BASE + 0x34),
    gpio_eds0 = (GPIO_BASE + 0x40),
    gpio_ren0 = (GPIO_BASE + 0x4C),
    gpio_fen0 = (GPIO_BASE + 0x58),
    // gpio_hen0 = (GPIO_BASE + 0x64),
    // gpio_len0 = (GPIO_BASE + 0x70),
    // gpio_pud  = (GPIO_BASE + 0x94),
    // gpio_pudclk0  = (GPIO_BASE + 0x98),

    INT_EN_2 = 0x2000B214 // (0x2000B214) Has gpio_int[0:3] for enabling

    // <you will need other values from BCM2835!>
};

// returns 1 if there is currently a GPIO_INT0 interrupt, 
// 0 otherwise.
//
// note: we can only get interrupts for <GPIO_INT0> since the
// (the other pins are inaccessible for external devices).
int gpio_has_interrupt(void) { // ** 
    // todo("implement: is there a GPIO_INT0 interrupt?\n");
    dev_barrier();

    // Read interrupt register Enable IRQs 2
    volatile unsigned value = GET32(INT_EN_2);
    return value & 1<<(49-32);
}

static void or32(volatile unsigned addr, uint32_t val) {
    device_barrier();
    PUT32(addr, get32(addr) | val);
    device_barrier();
}

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

    // Enable interrupt
    or32(INT_EN_2, 1 << (49 - pin));

    // Enable rising edge
    or32(gpio_ren0, 1 << pin); // Only gpio_ren0 (not gpio_ren1 too) because pin is less than 32

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

    // Enable interrupt
    or32(INT_EN_2, 1 << (49 - pin));

    // Enable rising edge
    or32(gpio_fen0, 1 << pin); // Only gpio_fen0 (not gpio_fen1 too) because pin is less than 32

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
    if (GET32(gpio_eds0) & (1 << pin)) {
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
    PUT32(gpio_eds0, 1 << pin);
    dev_barrier();
}


// // p98: detect when input pin=1.  must clear the source of the 
// // interrupt before clearing the event or it will just retrigger.
// void gpio_enable_hi_int(unsigned pin) {
//     volatile unsigned addr = (unsigned) gpio_hen0 + 4 * (pin / 32);
//     volatile unsigned value = GET32(addr);

//     PUT32(addr, value |= 1 << (pin % 32));
// }

// void gpio_int_low(unsigned pin) {
//     volatile unsigned addr = (unsigned) gpio_len0 + 4 * (pin / 32);
//     volatile unsigned value = GET32(addr);

//     PUT32(addr, value |= 1 << (pin % 32));
// }