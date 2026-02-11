/**
 * Definitions for all the registers and which pages to find them on
 */
#ifndef __RPI_REGS_H__
#define __RPI_REGS_H__

/**
 * GPIO Registers (Section 6, pg. 89)
 */

enum {
    GPIO_BASE = 0x20200000,

    GPFSEL0 = GPIO_BASE,                //  (0x20200000) GPIO Function Select 0 (p. 91)
    GPSET0 = (GPIO_BASE + 0x1C),        //  (0x2020001C) GPIO Pin Output Set 0 (p. 95)
    GPCLR0 = (GPIO_BASE + 0x28),        //  (0x20200028)  GPIO Pin Output Clear 0 (p. 95)
    GPLEV0 = (GPIO_BASE + 0x34),        //  (0x20200034)  GPIO Pin Level 0 (p. 96)
    GPEDS0 = (GPIO_BASE + 0x40),        //  (0x20200040)  GPIO Pin Event Detect Status 0 (p. 96)
    GPREN0 = (GPIO_BASE + 0x4C),        //  (0x2020004C)  GPIO Pin Rising Edge Detect Enable 0 (p. 97)
    GPFEN0 = (GPIO_BASE + 0x58),        //  (0x20200058)  GPIO Pin Falling Edge Detect Enable 0 (p. 98)
    GPHEN0 = (GPIO_BASE + 0x64),        //  (0x20200064)  GPIO Pin High Detect Enable 0 (p. 98)
    GPLEN0 = (GPIO_BASE + 0x70),        //  (0x20200070)  GPIO Pin Low Detect Enable 0 (p. 99)
    GPPUD = (GPIO_BASE + 0x94),         //  (0x20200094)  GPIO Pin Pull-up/down Enable (p. 100)
    GPPUDCLK0 = (GPIO_BASE + 0x98),     //  (0x20200098)  GPIO Pin Pull-up/down Enable Clock 0 (p. 101)

    // Aliases to what other things use
    gpio_set0  = GPSET0,                //  (0x2020001C) GPIO Pin Output Set 0 (p. 95)
    gpio_clr0  = GPCLR0,                //  (0x20200028)  GPIO Pin Output Clear 0 (p. 95)
    gpio_lev0  = GPLEV0,                //  (0x20200034)  GPIO Pin Level 0 (p. 96)
    gpio_eds0 = GPEDS0,                 //  (0x20200040)  GPIO Pin Event Detect Status 0 (p. 96)
    gpio_ren0 = GPREN0,                 //  (0x2020004C)  GPIO Pin Rising Edge Detect Enable 0 (p. 97)
    gpio_fen0 = GPFEN0,                 //  (0x20200058)  GPIO Pin Falling Edge Detect Enable 0 (p. 98)
    gpio_hen0 = GPHEN0,                 //  (0x20200064)  GPIO Pin High Detect Enable 0 (p. 98)
    gpio_len0 = GPLEN0,                 //  (0x20200070)  GPIO Pin Low Detect Enable 0 (p. 99)
    gpio_pud  = GPPUD,                  //  (0x20200094)  GPIO Pin Pull-up/down Enable (p. 100)
    gpio_pudclk0  = GPPUDCLK0,          //  (0x20200098)  GPIO Pin Pull-up/down Enable Clock 0 (p. 101)

    // <you will need other values from BCM2835!>
};

/**
 * Interrupt Register (Offsets) 
 */

enum {
    IRQ_PENDING_2 = 0x2000B208, // Has gpio_int[0:3]
    INT_EN_2 = 0x2000B214 // Has gpio_int[0:3]
};

#endif