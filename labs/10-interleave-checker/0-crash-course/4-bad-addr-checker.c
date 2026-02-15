// we modify the <0-nop-example.c> example, stripping out comments
// so its more succinct and wrapping up the diffent routines into
// a <single_step_fn> routine that will run a function with an 
// argument in single step mode.
#include "rpi.h"
#include "single-step.h"

static uint32_t addr = 0xDEADBEEF;

void bad_addr_writer() {
    
    PUT32(addr + 4, 0);
    PUT32(addr + 8, 0);
    PUT32(addr - 4, 0);
    PUT32(addr - 8, 0);
    PUT32(addr, 0);  // Should panic on this
}

// complete example for how to run single stepping on a simple routine.
void notmain(void) {
    kmalloc_init_mb(1);

    single_step_check_bad_addr(addr);

    PUT32(addr, 0xDEADBEEF);

    single_step_fn("bad_addr_writer", bad_addr_writer, 0, 0, 0);

}
