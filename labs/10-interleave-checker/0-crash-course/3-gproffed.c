// we modify the <0-nop-example.c> example, stripping out comments
// so its more succinct and wrapping up the diffent routines into
// a <single_step_fn> routine that will run a function with an 
// argument in single step mode.
#include "rpi.h"
#include "single-step.h"

static void single_stepping_this() {
    static volatile uint32_t val = 0; // Changes checksum (obv)
    for (uint32_t i = 0; i < 100; i++) {
        val += i;
    }

    /**
    0x8048: 1
    0x804c: 1
    0x8050: 1
    0x8188: 1   	e3a03000 	mov	r3, #0
    0x818c: 1   	ea000004 	b	81a4 <single_stepping_this+0x1c>
    0x8190: 100 	e59f1018 	ldr	r1, [pc, #24]	@ 81b0 <single_stepping_this+0x28>
    0x8194: 100 	e5912000 	ldr	r2, [r1]
    0x8198: 100 	e0822003 	add	r2, r2, r3
    0x819c: 100 	e5812000 	str	r2, [r1]
    0x81a0: 100 	e2833001 	add	r3, r3, #1
    0x81a4: 101 	e3530063 	cmp	r3, #99	@ 0x63
    0x81a8: 101 	9afffff8 	bls	8190 <single_stepping_this+0x8>
    0x81ac: 1   	e12fff1e 	bx	lr
    */
}

// complete example for how to run single stepping on a simple routine.
void notmain(void) {
    kmalloc_init_mb(1);

    gprof_init();

    single_step_gprof(1);

    single_step_fn("single_stepping_this", single_stepping_this, 0, 0, 0);
    single_step_fn("single_stepping_this", single_stepping_this, 0, 0, 0);

    gprof_dump(0);
}
