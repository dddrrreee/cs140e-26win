// part 7

#include "rpi.h"

void notmain(void) {
    enum { led = 27, input = 8};

    gpio_set_input(input);
    // gpio_set_pullup(input); // Change to pulldown to see 
    gpio_set_pulldown(input); 
    gpio_set_output(led);

    delay_cycles(1000000);

    unsigned v = 0; 

    while(1) {

        unsigned v = gpio_read(input);

        // Sets Parthiv LED to value of input pin
        // Should default to ON if nothing is connected (because pullup)
        // Must connect pin 8 to ground will turn the LED off
        gpio_write(led, v);
        
        delay_cycles(1000000);
    }
}
