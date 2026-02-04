// simple test to check we are handling rising edge interrupt
// correctly.
//
// NOTE: make sure you connect your GPIO 20 to your GPIO 21 (i.e., 
// have it "loopback")
#include "test-interrupts.h"

static void test_one_rising(void) {
    assert(!n_falling);
    assert(!n_interrupt);
    assert(gpio_read(in_pin) == 0);

    // checking <n_rising> right away only works b/c 
    // <gpio_write()> takes so long that we are guaranteed 
    // the interrupt fired.  Good exercise: measure how
    // long the interrupt takes to propagate!

    assert(!n_rising);
    gpio_write(out_pin, 1);
    if(!n_rising)
        panic("rising edge wrong: %d\n", n_rising);
    assert(n_rising == 1);
    assert(n_interrupt == 1);
    trace("test passed: got a rising edge!\n");
}

void notmain() {
    trace("test: single rising edge detection.\n");
    // see: <tests-interrupt.c>
    rising_init();
    test_one_rising();
    trace("SUCCESS: test passed!\n");
}
