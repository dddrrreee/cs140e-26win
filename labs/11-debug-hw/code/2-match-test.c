// simple breakpoint test:
//  1. set a single breakpoint on <foo>.
//  2. in exception handler, make sure the fault pc = <foo> and disable
//     the breakpoint.
//  3. if that worked, do 1&2 <n> times with get32/put32 to make 
//     sure works.
#include "rpi.h"
#include "breakpoint.h"
#include "full-except.h"

// the routine we fault on: we don't want to use GET32 or PUT32
// since that would break printing.
void foo(int x);

// for error checking: record the match we expected to see.
static volatile uint32_t expected_pc;

// total number of faults.
static volatile int n_faults = 0;

static void brkpt_fault(regs_t *r) {
    if(!brkpt_fault_p())
        panic("should only get a breakpoint fault\n");

    // disable first so we can fault on put32/get32
    // Q: if you delete: why does the test lock up?
    brkpt_match_stop();
    if(brkpt_match_get())
        panic("match should be disabled\n");

    // see: 13-34: effect of exception on watchpoint registers.
    // afaik: have to check the pc to see if it was a match.
    uint32_t pc = r->regs[15];
    trace("\tfault pc=[%x] expect=[%x]\n", pc, expected_pc);
    if(pc != expected_pc)
        panic("expected match on %p, have %p\n", pc, expected_pc);

    // increment fault count, disable the fault and jump back.
    n_faults++;

    // switch back and run the faulting instruction.
    switchto(r);
}

void notmain(void) {
    // 1. install exception handlers.
    full_except_install(0);
    full_except_set_prefetch(brkpt_fault);


    // 2. enable the debug system.
    brkpt_match_init();

    // 3. record where we expect to fault.
    expected_pc = (uint32_t)foo;
    output("set breakpoint for addr %p\n", expected_pc);
    brkpt_match_set(expected_pc);

    // 4. take a fault.
    output("about to call %p: should see a fault!\n", foo);
    foo(0);
    assert(n_faults == 1);

    // 5. take a bunch of faults.   alternate between
    //    GET32 and PUT32
    int n = 10;
    trace("worked!  fill do %d repeats\n", n);
    for(int i = n_faults = 0; i < n; i++) {
        uint32_t x;
        expected_pc = (uint32_t)put32;
        brkpt_match_set(expected_pc);
        if(brkpt_match_get() != expected_pc)
            panic("match not set?\n");

        trace("should see a breakpoint fault!\n");
        put32(&x, i);
        assert(x == i);
        if(brkpt_match_get())
            panic("exception handler should clear match\n");

        expected_pc = (uint32_t)get32;
        brkpt_match_set(expected_pc);

        if(get32(&x) != i)
            panic("get32 didn't run!\n");
        if(brkpt_match_get())
            panic("exception handler should clear match\n");

        trace("n_faults=%d, i=%d\n", n_faults,i);
        assert(n_faults == (i+1)*2);
    }
    trace("SUCCESS\n");
}

// weak attempt at preventing inlining.
void foo(int x) {
    trace("running foo: %d\n", x);
}
