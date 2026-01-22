// trivial redzone test: corrupt a couple wods and make sure get detected
#include "test-header.h"

// trivial first thread: does not block, returns.
// makes sure you pass in the right argument.
static void thread_code(void *arg) {
    unsigned *x = arg;

    // check tid
    rpi_thread_t *t = rpi_cur_thread();

	trace("in thread [addr=%p], tid=%d with x=%x\n", t, t->tid, *x);
    demand(rpi_cur_thread()->tid == *x, 
                "expected %d, have %d\n", t->tid,*x+1);

    *x += 1;
}

void notmain() {
    test_init();

    output("------------------------------------------------\n");
    redzone_check("before corrupt: expect no errors");

    output("------------------------------------------------\n");
    output("we expect two ERRORs at addr=0, and addr=8\n");

    // corrupt addr=0, addr=8
    PUT32(0x0, 1);
    PUT32(0x8, 1);
    redzone_check("should see corruption!");

    not_reached();
}
