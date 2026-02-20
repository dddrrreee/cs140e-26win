// engler, 140e'26: simple test to check that you restore 
// user-mode registers correctly
//   - big problem: if your restore code has a bug, the code
//     often usually jumps into a big black hole you will have
//     no visibility into.  
//   - very hard to debug since anything you do will perturb 
//     the state (often can't do anything but stare).
//   - our cute hack: abuse mis-matching!
//   - make checking extremely simple, few moving parts.
//
// basic idea:
//   1. set the 17-entry register block to arbitrary values.
//   2. set a mismatch on an impossible address.
//   3. when restore <regs> and cause a jump to user
//      mode will immediately get a mismatch fault *before* 
//      doing anytihng.
//   4. in the mismatch handler (prefetch abort) check the 
//      registers.  
//        - if they are what you expect, both save/restore
//          worked.
//        - if not, you have a bug: easy to iterate since
//          not much code.
//
// what you write:
//   1. take the trampoline from last step and add it to 
//      prefetch abort handler in <start.S>
//   2. write the restore code in <switchto_user_regs_asm>
//      (also in <start.S>).
//
// NOTE:
//   - if your save registers trampoline is broken from last 
//     step, will be hard to debug -- so make sure it works.
//   - this is not a *verification* but it is easy to debug
//     piece-by-piece.  next step does interleaving.
#include "rpi.h"
#include "breakpoint.h"
#include "cpsr-util.h"
#include "switchto.h"
#include "vector-base.h"

// used to check that the registers that we used
// are what we got.
static uint32_t expected_regs[17];

// check that the cpsr mode is what we expected
static void check_cpsr(uint32_t expected_mode) {
    // sanity check that we are at SUPER_MODE.
    // (this should not fail).
    uint32_t cpsr = cpsr_get();
    uint32_t cpsr_mode = mode_get(cpsr);
    if(cpsr_mode != ABORT_MODE)
        panic("expected mode=%b: have %b\n", 
                ABORT_MODE, cpsr_mode);
}

// check that the spsr mode is what we expected
static void check_spsr(uint32_t expected_mode) {
    // better have come from user mode or some problem
    // with your rfe code.
    uint32_t spsr = spsr_get();
    uint32_t spsr_mode = mode_get(spsr);
    if(spsr_mode != USER_MODE)
        panic("expected mode=%b: have %b\n", 
                USER_MODE, spsr_mode);
}

// check that all 17 entries in <regs> match what we 
// expected <expect>
static int 
check_regs(uint32_t regs[17], uint32_t expect[17]) {
    // check that the registers are set correctly. 
    // see: <start.S:regs_init_ident>
    for(int i = 0; i < 17; i++)  {
        if(regs[i] != expect[i])
            panic("reg[%d] = %x, expected = %x\n", 
                regs[i], expect[i]);
        trace("regs[%d]=%d\n", i, regs[i]);
    }
    // should be redundant, but check that saved <cpsr> == <spsr>
    let spsr = spsr_get();
    if(regs[16] != spsr)
        panic("spsr saved incorrectly: spsr=%x, saved=%x\n",
            spsr, regs[16]);

    trace("regs matched!\n");
    return 1;
}

// a one-shot system call to debug the values you set to.
// you need to modify the system call trampoline 
// in <start.S> so that it passes the 17-entry registers in.
// 
// we do it this way b/c much easier to debug with not
// many moving pieces.
void prefetch_abort_handler(uint32_t regs[17]) {
    // same as last test.
    check_cpsr(ABORT_MODE);
    check_spsr(USER_MODE);
    // verify <regs> == <expected_regs>
    check_regs(regs, expected_regs);
    trace("success!\n");
    clean_reboot();
}

void notmain(void) {
    // setup vector see <interrupt-asm.S>
    extern uint32_t simple_reg_save_restore_vec[];
    vector_base_set(simple_reg_save_restore_vec);

    // initialize registers to known values.
    // we dup <regs> and <expected_regs> in case
    // the <switchto_user_regs_asm> has a corruption 
    // bug.
    uint32_t regs[17];
    for(int i = 0; i < 15; i++)
        regs[i] = expected_regs[i] = i;
    // pc doesn't matter as long as 4 byte aligned.
    regs[15] = expected_regs[15] = 0x1010;
    regs[16] = expected_regs[16] = USER_MODE;

    enum { impossible_pc = ~0 << 2 };
    // mismatch on an impossible address that isn't null.
    // should trap before running.
    brkpt_mismatch_start();
    brkpt_mismatch_set(impossible_pc);
    // you write this: load and jump to <regs>
    switchto_user_asm((void*)regs);  // sorry: gross cast
    not_reached();
}
