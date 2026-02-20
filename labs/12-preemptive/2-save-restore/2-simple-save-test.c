// simple test to check that you save registers correctly
//   - you only have to write the save code in the syscall
//     trampoline (in: interrupt-asm.S)
//   - this is not a *verification* but it is easy to debug
//     piece-by-piece: next test does the restore.
//
// basic idea:
//  1. <notmain> uses your <rfe_asm> routine to run 
//     <start.S:regs_init_ident> at user level.
//  2. <regs_init_ident> sets all the registers to known 
//     values and then does a system call instruction <swi>
//  3. TODO: system call trampoline: you write the code to 
//     save the 16 general registers, and the cpsr in 
//     a 17 entry array (on the stack) and pass it in.
//  4. <system_call_handler> verifies the register array.
//     has the right values and exits.  (i.e., it is
//     one-shot).
//
//  If you look at <start.S:regs_init_ident> it sets
//  all registers besides cpsr and pc to their name: 
//      MK_FN(regs_init_ident)
//          mov r0, #0
//          mov r1, #1
//          mov r2, #2
//          mov r3, #3
//          mov r4, #4
//          mov r5, #5
//          mov r6, #6
//          mov r7, #7
//          mov r8, #8
//          mov r9, #9
//          mov r10, #10
//          mov r11, #11
//          mov r12, #12
//          mov r13, #13
//          mov r14, #14
//          swi 1
//          @ exception lr will be here.
//      .global regs_init_ident_swi_pc
//      regs_init_ident_swi_pc:
//          asm_not_reached();
#include "rpi.h"
#include "breakpoint.h"
#include "cpsr-util.h"
#include "switchto.h"
#include "vector-base.h"


// a one-shot system call to debug the values you set to.
// you need to modify the system call trampoline 
// in <start.S> so that it passes the 17-entry registers in.
// 
// we do it this way b/c much easier to debug with not
// many moving pieces.
int syscall_handler(uint32_t regs[17]) {
    // sanity check that we are at SUPER_MODE.
    // (this should not fail).
    uint32_t cpsr = cpsr_get();
    uint32_t cpsr_mode = mode_get(cpsr);
    if(cpsr_mode != SUPER_MODE)
        panic("expected mode=%b: have %b\n", 
                SUPER_MODE, cpsr_mode);

    // better have come from user mode or some problem
    // with your rfe code.
    uint32_t spsr = spsr_get();
    uint32_t spsr_mode = mode_get(spsr);
    if(spsr_mode != USER_MODE)
        panic("expected mode=%b: have %b\n", 
                USER_MODE, cpsr_mode);

    // check that the registers are set correctly. 
    // see: <start.S:regs_init_ident>
    for(int i = 0; i < 15; i++)  {
        trace("regs[%d]=%d\n", i, regs[i]);
        assert(regs[i] == i);
    }
    trace("regs[15]=%x\n", regs[15]); // pc
    trace("regs[16]=%x\n", regs[16]); // cpsr

    // see <start.S>: label that holds pc of where the swi 
    // instruction should return to.
    extern uint32_t regs_init_ident_swi_pc[];
    uint32_t expected_pc = (uint32_t)regs_init_ident_swi_pc;
    uint32_t pc = regs[15];
    if(pc != expected_pc)
        panic("expected pc=%x, have %x\n", expected_pc,pc);

    // check that saved <cpsr> == <spsr>
    if(regs[16] != spsr)
        panic("spsr saved incorrectly: spsr=%x, saved=%x\n",
            spsr, regs[16]);

    trace("success!\n");
    clean_reboot();
}

void notmain(void) {
    // setup vector see <interrupt-asm.S>
    extern uint32_t syscall_reg_save_vec[];
    vector_base_set(syscall_reg_save_vec);

    // use your <rfe_asm> to run <regs_init_ident> 
    // at user level.
    void rfe_asm(uint32_t regs[2]);
    uint32_t regs[2];

    // see <start.S>
    void regs_init_ident(void);

    regs[0] = (uint32_t)regs_init_ident;
    regs[1] = USER_MODE;
    rfe_asm(regs);

    not_reached();
}
