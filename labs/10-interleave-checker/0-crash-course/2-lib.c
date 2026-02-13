// we modify the <0-nop-example.c> example, stripping out comments
// so its more succinct and wrapping up the diffent routines into
// a <single_step_fn> routine that will run a function with an 
// argument in single step mode.
#include "rpi.h"
#include "breakpoint.h"
// #include "full-except.h"
// #include "cpsr-util.h"
// #include "single-step-syscalls.h"
#include "single-step.h"

// complete example for how to run single stepping on a simple routine.
void notmain(void) {
    kmalloc_init_mb(1);

    // make a single stack at the same address for everyone.
    enum { stack_size = 64 * 1024 };
    void *stack = kmalloc(stack_size);
    
    // regs_t r = {};
    trace("******************<nop_10>******************************\n");

    void nop_1(void);
    void nop_10(void);
    void mov_ident(void);
    void hello_asm(void);
    // r = single_step_fn("nop_10", nop_10, 0, 0, 0);
    // single_step_verbose(0);
    single_step_fn("nop_1", nop_1, 0, 0, 0);
    single_step_fn("nop_10", nop_10, 0, 0, 0);
    single_step_fn("mov_ident", mov_ident, 0, 0, 0);
    single_step_fn("hello_asm", hello_asm, 0, stack, stack_size);

    // // this routine used a trampoline: check that the final pc
    // // is correct.
    // extern uint32_t exit_tramp_pc[]; // see <single-step-start.S>
    // trace("expect: pc should be %x!\n", exit_tramp_pc);
    // void *exit_pc = (void *)r.regs[15];
    // if(exit_pc != exit_tramp_pc)
    //     panic("final pc should be = %x, is %x!\n", exit_tramp_pc, exit_pc);
    // trace("success: exit pc=%x!\n", exit_pc);
}
