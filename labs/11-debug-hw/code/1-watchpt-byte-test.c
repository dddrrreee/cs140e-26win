/*
 * Simple test that:
 *  1. sets a watchpoint on null (0).
 *  2. does stores on 0x0, 0x1, 0x2, 0x3
 *  3. checks that (2) caused the right fault.
 *  4. does a load on 0x0, 0x1, 0x2, 0x3
 *  5. checks that (4) caused a fault and returned val.
 *
 *
 * shows how to use the full_except/switchto interface.
 */
// handle a store to address 0 (null)
#include "rpi.h"
#include "vector-base.h"
#include "watchpoint.h"
#include "full-except.h"

static volatile int load_fault_n, store_fault_n;

// the fault address and pc we expect.
static uint32_t expect_addr, expect_pc, expect_load_p;

// change to passing in the saved registers.

static void watchpt_fault(regs_t *r) {
    if(!watchpt_fault_p())
        panic("should only get debug faults!\n");

    // we get the fault *after* the read or write occurs and
    // the pc value is the next instruction that would run.
    // e.g., a store or a load to a non-pc value expect this
    // to be +4.    if they write to the pc can be anywhere.
    uint32_t pc = r->regs[15];

    // the actual instruction that caused watchpoint.  pc holds the
    // address of the next instruction.
    uint32_t fault_pc = watchpt_fault_pc();

    // these can differ in your code
    trace("\t1. fault pc=[%x], resume pc=[%x]\n", fault_pc, pc);

    // fault pc should be what we expected.
    if(fault_pc != expect_pc)
        panic("expected fault at pc=%p, have fault_pc = %p\n", 
            expect_pc, fault_pc);

    // check that its load/store as expected
    if(watchpt_load_fault_p()) {
        assert(expect_load_p == 1);
        trace("\t2. load fault at pc=[%x]\n", fault_pc);
        load_fault_n++;
    } else {
        assert(expect_load_p == 0);
        trace("\t2. store fault at pc=[%x]\n", fault_pc);
        store_fault_n++;
    }

    // check that the address matches.
    uint32_t addr = watchpt_fault_addr();
    trace("\t3. faulting addr = %p\n", addr);
    if(addr != expect_addr)
        panic("expected fault on addr=%p, have %p\n", expect_addr, addr);

    // disable the watchpoint.
    watchpt_off(addr);
    trace("\t4. disabled %p\n", addr);

    // resume.
    switchto(r);
}

static void do_put8_test(uint32_t addr, uint8_t val) {
    expect_pc = (uint32_t)PUT8;
    expect_addr = addr;
    expect_load_p = 0;

    // set watchpoint.
    trace("setting watchpoint for addr %p\n", addr);
    watchpt_on(addr);

    int orig_n = store_fault_n;
    trace("about to call PUT8: should fault\n");
    PUT8(addr, val);
    trace("afer PUT8: store_fault=%d!\n", store_fault_n);
    assert(GET8(addr) == val);

    if(store_fault_n != orig_n+1)
        panic("should see exactly one store fault: have%d\n", 
                store_fault_n-orig_n);
}

void notmain(void) {
    // initialize null.
    PUT32(0x0, 0xdeadbeef);

    // install exception handlers: see <staff-full-except.c>
    full_except_install(0);
    full_except_set_data_abort(watchpt_fault);

    // do put8's to 0x0, 0x1, 0x2, 0x3
    // if these don't work you are not handling sub-word writes
    do_put8_test(0x0, 1);
    do_put8_test(0x1, 2);
    do_put8_test(0x2, 3);
    do_put8_test(0x3, 4);

    trace("SUCCESS GET8 fault worked!\n");
}
