// tests that your gpio_input works.
// hardware:
//   1. connect pin 18 and 19 using a jumper ("loopback").
//   2. leave your LEDs connected to 20 and 21.
//   3. if the test succeeds they should blink in unison.  either
//      both on or both off.
// 
// we do "true" embedded system debugging without any printk
// available by using LEDs to show what the system is doing:
//  iterate, alternating the value of <v> \in {0,1}
//  1. set LED on 20 to current value of <v> (ground truth)
//  2. set pin 18 (the loopback output) to <v>   
//  3. read pin 19 (the loopback input) and set pin 21 
//     to this value.
//
//  - if both LEDs match (both on at the same time, both off
//    at the same time) you are correct.  
//   - if not: first try the staff binary to make sure your 
//     pi is wired correctly.
#include "rpi.h"

void notmain(void) {
    const int led0 = 20;
    const int led1 = 21;
    const int output = 9;
    const int input = 10;

    gpio_set_output(led0);
    // do in the middle in case some interference
    gpio_set_input(input);
    gpio_set_output(led1);
    gpio_set_output(output);

    unsigned v = 0;
    for(int i = 0; i < 20; i++) {
        // 1. set pin <led0> to <v> so we have something 
        // to compare to.
        gpio_write(led0, v);

        // 2. write <v> to the loopback, read it, then
        //    set <led1> to the value.  visually: <led1>
        //    should be the same as <led0> (either on or off).
        gpio_write(output, v);
        gpio_write(led1, gpio_read(input));

        // hold so can see visually (humans are slow)
        delay_cycles(1500000);

        // flip the value of v from 0->1 or 1->0
        // note: could also do  <v> = 1-<v>
        v = !v; 
    }
}
