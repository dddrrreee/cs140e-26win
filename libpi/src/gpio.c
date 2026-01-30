/*
 * Implement the following routines to set GPIO pins to input or 
 * output, and to read (input) and write (output) them.
 *  1. DO NOT USE loads and stores directly: only use GET32 and 
 *    PUT32 to read and write memory.  See <start.S> for thier
 *    definitions.
 *  2. DO USE the minimum number of such calls.
 * (Both of these matter for the next lab.)
 *
 * See <rpi.h> in this directory for the definitions.
 *  - we use <gpio_panic> to try to catch errors.  For lab 2
 *    it only infinite loops since we don't have <printk>
 */
#include "rpi.h"
#include "gpio.h"

// macro hack
#define gpio_panic(msg...) panic(msg)

// See broadcomm documents for magic addresses and magic values.
//
// If you pass addresses as:
//  - pointers use put32/get32.
//  - integers: use PUT32/GET32.
//  semantics are the same.
enum {
    // Max gpio pin number.
    GPIO_MAX_PIN = 53,
    GPIO_MAX_FUNCTION = 7,

    GPIO_BASE = 0x20200000,
    gpio_set0  = (GPIO_BASE + 0x1C),
    gpio_clr0  = (GPIO_BASE + 0x28),
    gpio_lev0  = (GPIO_BASE + 0x34),
    gpio_pud  = (GPIO_BASE + 0x94),
    gpio_pudclk0  = (GPIO_BASE + 0x98)

    // <you will need other values from BCM2835!>
};

#define PULLDOWN_WAITTIME 150 // cycles

//
// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output
//

// set <pin> to be an output pin.
//
// NOTE: fsel0, fsel1, fsel2 are contiguous in memory, so you
// can (and should) use ptr calculations versus if-statements!
void gpio_set_output(unsigned pin) {
//     if(pin > GPIO_MAX_PIN)
//         gpio_panic("illegal pin=%d\n", pin);

//   // Implement this.
//     volatile unsigned* gpio_addr = (unsigned*) GPIO_BASE + (pin / 10);
//     volatile unsigned mask_n = 0b111 << (3 * (pin % 10));
//     volatile unsigned value = get32(gpio_addr);

//     value &= ~mask_n;
//     value |= 0b001 << (3 * (pin % 10));

//     put32(gpio_addr, value);
    gpio_set_function(pin, GPIO_FUNC_OUTPUT);
}

// Set GPIO <pin> = on.
void gpio_set_on(unsigned pin) {
    if(pin > GPIO_MAX_PIN)
        gpio_panic("illegal pin=%d\n", pin);

    // Implement this. 
    // NOTE: 
    //  - If you want to be slick, you can exploit the fact that 
    //    SET0/SET1 are contiguous in memory.
    volatile unsigned* gpio_addr = (unsigned*) gpio_set0 + (pin / 32);
    put32(gpio_addr, 1 << (pin % 32));
}

// Set GPIO <pin> = off
void gpio_set_off(unsigned pin) {
    if(pin > GPIO_MAX_PIN)
        gpio_panic("illegal pin=%d\n", pin);

    // Implement this. 
    // NOTE: 
    //  - If you want to be slick, you can exploit the fact that 
    //    CLR0/CLR1 are contiguous in memory.
    volatile unsigned* gpio_addr = (unsigned*) gpio_clr0 + (pin / 32);
    put32(gpio_addr, 1 << (pin % 32));
}

// Set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
    if(v)
        gpio_set_on(pin);
    else
        gpio_set_off(pin);
}

//
// Part 2: implement gpio_set_input and gpio_read
//

// set <pin> = input.
void gpio_set_input(unsigned pin) {
    // if(pin > GPIO_MAX_PIN)
    //     gpio_panic("illegal pin=%d\n", pin);

    // // Implement.
    // volatile unsigned* gpio_addr = (unsigned*) GPIO_BASE + (pin / 10);
    // volatile unsigned mask_n = 0b111 << (3 * (pin % 10));
    // volatile unsigned value = get32(gpio_addr);

    // value &= ~mask_n;
    // // value |= 0b000 << (3 * (pin % 10)); don't need to set them for input

    // put32(gpio_addr, value);

    gpio_set_function(pin, GPIO_FUNC_INPUT);
}

// Return 1 if <pin> is on, 0 if not.
int gpio_read(unsigned pin) {
    if(pin > GPIO_MAX_PIN)
        gpio_panic("illegal pin=%d\n", pin);

    volatile unsigned* gpio_addr = (unsigned*) gpio_lev0 + (pin / 32);
    volatile unsigned value = get32(gpio_addr);

    return (value >> (pin % 32)) & 0b1;
}


//
// Part 3: implement gpio_set_pullup and gpio_set_pulldown
//

void gpio_set_pullup(unsigned pin) {
    if(pin > GPIO_MAX_PIN)
        gpio_panic("illegal pin=%d\n", pin);

    // Implement this.
    
    // Write to GPPUD to enable control line and wait for the control line to set-up (?) 
    put32((unsigned*)gpio_pud, 0b10);
    delay_cycles(PULLDOWN_WAITTIME);

    // Write to GPPUDCLK0/1 to "Assert Clock on line (n)"
    volatile unsigned* pud_ck_addr = (unsigned*) gpio_pudclk0 + (pin / 32);
    put32(pud_ck_addr, 1 << (pin % 32));
    delay_cycles(PULLDOWN_WAITTIME); // Iffy on whether we have to wait again (according to annotations on datasheet)

    // Write back to GPPUD 
    put32((unsigned*)gpio_pud, 0);
    put32(pud_ck_addr, 0);
}


void gpio_set_pulldown(unsigned pin) {
    if(pin > GPIO_MAX_PIN)
        gpio_panic("illegal pin=%d\n", pin);

    // Implement this.
    
    put32((unsigned*)gpio_pud, 0b01);
    delay_cycles(PULLDOWN_WAITTIME);

    volatile unsigned* pud_ck_addr = (unsigned*) gpio_pudclk0 + (pin / 32);
    put32(pud_ck_addr, 1 << (pin % 32));
    delay_cycles(PULLDOWN_WAITTIME); // Iffy on whether we have to wait again (according to annotations on datasheet)

    put32((unsigned*)gpio_pud, 0);
    put32(pud_ck_addr, 0);
}

//
// Part 3: implement gpio_set_function
//


// void gpio_set_function(unsigned pin, gpio_func_t function) {
    
//     unsigned masked_value = (unsigned)function << (3 * (pin % 20));

//     volatile unsigned* gpio_addr = (unsigned*)gpio_set0 + pin / 20;

//     unsigned mask = 0b111 << (3 * (pin % 20));

//     unsigned value = get32(gpio_addr);
//     put32(gpio_addr, (value & mask) | masked_value);
// }

void gpio_set_function(unsigned pin, gpio_func_t function) {
    if(pin > GPIO_MAX_PIN)
        gpio_panic("illegal pin=%d\n", pin);

    if((function & 0b111) != function)
        gpio_panic("illegal func=%x\n", function);

  // Implement this.
    volatile unsigned int gpio_addr = (unsigned) GPIO_BASE + 4 * (pin / 10);
    volatile unsigned int mask_n = 0b111 << (3 * (pin % 10));
    volatile unsigned int value = GET32(gpio_addr);

    value &= ~mask_n;
    value |= function << (3 * (pin % 10));

    PUT32(gpio_addr, value);
}