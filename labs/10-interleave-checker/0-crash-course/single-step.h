#ifndef __SINGLE_STEP_H__
#define __SINGLE_STEP_H__

#include <stdint.h>
#include "rpi.h"
#include "switchto.h"


void single_step_verbose(int v_p);

// single-step (mismatch) breakpoint handler:
void single_step_handler(regs_t *regs);

// single system call
int syscall_handler(regs_t *r);

/*****************************************************************
 * standard code to create the initial thread registers.
 */
// make a user-level <cpsr> based on the 
// current <cpsr> value
static inline uint32_t cpsr_to_user(void) {
    // inherit whatever configured with.
    uint32_t cpsr = cpsr_get();

    cpsr = bits_clr(cpsr, 0, 4) | USER_MODE;
    // clear carry flags (non-determ values)
    cpsr = bits_clr(cpsr, 28, 31);
    // enable interrupts (doesn't matter here).
    cpsr = bit_clr(cpsr, 7);

    assert(mode_get(cpsr) == USER_MODE);
    return cpsr;
}

// create a new register block: we allow for a null <stack>
// for ease of testing.
static inline regs_t 
regs_init(
    void (*fp)(), 
    uint32_t arg, 
    void *stack, 
    uint32_t nbytes) {

    assert(fp);
    uint32_t initial_pc = (uint32_t)fp; 
    uint32_t cpsr = cpsr_to_user();

    uint32_t sp = 0;
    if(stack) {
        demand(nbytes>4096, stack seems small);
        sp = (uint32_t)(stack+nbytes);
    }

    // better to pass it in, but we try to be simple.
    void exit_trampoline(void);
    uint32_t exit_tramp = (uint32_t)exit_trampoline;

    // <regs_t>: 17 uint32_t entries (see: <switchto.h>
    //  - 16 general purpose regs r0..r15 (r0 at r.regs[0], 
    //    r1 at r.regs[1], etc).
    //  - the cpsr at r.regs[16].
    return (regs_t) { 
        .regs[REGS_PC] = initial_pc,
        .regs[REGS_R0] = arg,
        .regs[REGS_SP] = sp,
        .regs[REGS_CPSR] = cpsr,
        // where to jump to if the code returns.
        .regs[REGS_LR] = (uint32_t)exit_tramp,
    };
}

void single_step_on(void);
void single_step_off(void);

// run function <fp> in single stepping mode.
regs_t single_step_fn(
    const char *fn_name, 
    void (*fp)(), 
    uint32_t arg,
    void *stack,
    uint32_t nbytes);

#endif
