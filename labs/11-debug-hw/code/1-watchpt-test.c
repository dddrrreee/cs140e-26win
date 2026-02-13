/*
 * Simple test that:
 *  1. sets a watchpoint on null (0).
 *  2. does a store using PUT32(0, val)
 *  3. checks that (2) caused the right fault.
 *  4. does a load using GET32(0)
 *  5. checks that (4) caused a fault and returned val.
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

    trace("\t1. fault pc=%x, resume pc=%x\n", fault_pc, pc);

    // fault pc should be what we expected.
    if(fault_pc != expect_pc)
        panic("expected fault at pc=%p, have fault_pc = %p\n", 
            expect_pc, fault_pc);

    // check that its load/store as expected
    if(watchpt_load_fault_p()) {
        assert(expect_load_p == 1);
        trace("\t2. load fault at pc=%x\n", fault_pc);
        load_fault_n++;
    } else {
        assert(expect_load_p == 0);
        trace("\t2. store fault at pc=%x\n", fault_pc);
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

void notmain(void) {
    enum { null = 0 };
    // initialize null.
    PUT32(null, 0xdeadbeef);
    trace("INITIAL VALUE: addr %x = %x\n", null, GET32(null));

    // install exception handlers: see <staff-full-except.c>
    full_except_install(0);
    full_except_set_data_abort(watchpt_fault);

    //**************************************************************
    // 1. Test that we can catch stores.  To make sure we have
    //    a known fault pc and addr we use PUT32 to store a 
    //    known value <val> to null.
    // 
    //    So, we expect to get a fault on the first instruction
    //    of PUT32.   From your .list:
    //
    // 00008024 <PUT32>:
    //  8024:   e5801000    str r1, [r0]
    //  8028:   e12fff1e    bx  lr
    // so fault pc should be 0x8024, fault addr should be 0x0.
    trace("\n****************<PUT32 test>*****************************\n");
    expect_pc = (uint32_t)PUT32;
    expect_addr = null;
    expect_load_p = 0;

    // set watchpoint.
    trace("setting watchpoint for addr %p\n", null);
    watchpt_on(null);

    uint32_t val = 0xfaf0faf0;
    trace("about to PUT32(%p,%x): should see a fault!\n",
                null, val);
    PUT32(null,val);
    trace("afer PUT32: store_fault=%d!\n", store_fault_n);
    if(store_fault_n != 1)
        panic("should see exactly one store fault: have%d\n", 
        store_fault_n);

    // note: fault handler disabled watchpoint so this should 
    // not fault.
    // 
    // a bigger note: watchpoints only fault after the load/store
    // happens.
    uint32_t x = GET32(null);
    if(x == val)
        trace("SUCCESS: correctly got a fault on addr=%x, val=%x\n", 
                                null, x);
    else
        panic("returned %x, expected %x\n", x, val);

    /**************************************************************
     * 2. Test that we can catch loads.  To make sure we have
    //    a known fault pc and addr we use GET32 to load from <null> 
    //    Should get the same value <val> stored above.
    // 
    //    So, we expect to get a fault on the first instruction
    //    of GET32.   From your .list:
    //
    // 0000802c <GET32>:
    //      802c:   e5900000    ldr r0, [r0]
    //      8030:   e12fff1e    bx  lr
    //
    // so fault pc should be 0x802c, fault addr should be 0x0.
     */
    trace("\n****************<GET32 test>*****************************\n");
    expect_pc = (uint32_t)GET32;
    expect_addr = null;
    expect_load_p = 1;

    // set up the fault again.
    trace("setting watchpoint for addr %p\n", null);
    watchpt_on(null);

    trace("should see a load fault at pc=%x!\n", expect_pc);
    x = GET32(null);
    if(!load_fault_n)
        panic("did not see a load fault\n");
    if(x != val)
        panic("expected %x, got %x\n", val,x);
    trace("SUCCESS GET32 fault worked!\n");
}
