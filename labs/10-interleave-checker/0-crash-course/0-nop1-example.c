// a one-stop-shop complete example of how to use the armv6
// debug hardware to do single stepping execution to trap on
// each instruction in a routine.  we have use three routines:
//  1. brkpt_mismatch_start(): start mismatching.  this won't
//     have any effect until you switch to user mode.
//  2. brkpt_mismatch_set(pc): throw a mismatch exception (a
//     type of prefetch_abort exception) when the hardware 
//     sets the program counter value to anything other than 
//     <pc>.
//  3.  brkpt_mismatch_stop(): turn off mismatching.
//
// mismatch faults let you single-step one instruction by:
//  1. setting the pc = to that instruction address (in privileged
//     mode).
//  2. jumping to that instruction (at user mode).
//  3. the CPU will run that instruction.
//  4. when the pc advances to any other address, the CPU will
//     generate a prefetch abort fault.
//  5. in the fault handler, you can do step (1) again, or something 
//     else.
//
// alot of the pieces are old friends in new clothes:
//   1. the full context-switching routine <switchto_cswitch>
//      works similarly to your <rpi_cswitch> routine (lab 5)
//      except:
//        - it saves/restores all 16 general purpose registers and 
//          cpsr instead of only saving callee-saved registers 
//          (and ignoring <cspr> and caller-saved registers).
//        - it can switch between modes (user to supervisor, supervisor
//          to user, etc), rather than staying at the same level.
//   2. the more specialized version <switchto> is one-half of the 
//      context switch, which just loads the passed in register block.
//      we use this when we won't return to the switching /callsite.
//   3. we use an exception handler (similar to lab 4) but instead
//      of timer/device interrupts we handle prefetch/debug exceptions.
//   4. we use system calls to exit user mode (similar lab 4's
//      trivial system call example) but takes the full set of
//      registers and doesn't have to return back to the callsite
//      we came from.
//
// in all these: we use the full set of 17 registers needed to 
// completely describe the armv6 execution state.
//
// note: we write the examples in assembly (see <single-step-start.S>
// so that we have complete control over what is executed so we can
// sanity check it.
//
#include "rpi.h"
#include "breakpoint.h"
#include "full-except.h"
#include "cpsr-util.h"
#include "single-step-syscalls.h"

#undef trace
#define trace trace_nofn

// count how many instructions.
static volatile unsigned n_inst;

// initial registers: what we <switchto> to resume.
static regs_t scheduler_regs;
// hack to record the registers at exit for printing.
static regs_t exit_regs;

// see <single-step-start.S>: exit trampoline just like
// your threads package.
void exit_trampoline(void);

// single-step (mismatch) breakpoint handler: 
//  1. does sanity checking
//  2. prints the instruction count and pc.
//  3. sets up the next mismatch exception.
//  4. switches back to <regs>
static void single_step_handler(regs_t *regs) {
    // 1. Sanity checking.
    if(interrupts_on_p())
        panic("bad: kernel interrupts are enabled!\n");
    if(!brkpt_fault_p())
        panic("impossible: should get no other faults\n");
    // verify faulting pc was at user-level.
    assert(mode_get(regs->regs[REGS_CPSR]) == USER_MODE);
    // verify we are currently running at prefetch-abort level.
    assert(mode_get(cpsr_get()) == ABORT_MODE);

    // 2. print:
    //   - number of instructions trapped, 
    //   - faulting pc
    //   - the machine code at the faulting address.
    uint32_t pc = regs->regs[15];
    trace_nofn("fault:\t%X:\t%X  @ %d\n", pc, GET32(pc), ++n_inst);

    // handle uart race condition: the single-step
    // exception can happen anywhere including b/n
    // the uart code checking that there was space
    // on the TX FIFO and writing it, so make sure
    // there is space.  a rare race, except as soon
    // as you print and start dropping bytes. :)
    while(!uart_can_put8())
        ;

    // 3. setup the next mismatch exception.
    //
    // tell the armv6 debug hardware to fault on 
    // any instruction address besides <pc>: the
    // result:
    //  1. we will the instruction at <pc> without
    //     faulting.
    //  2. we will fault when it goes to the next
    //     instruction.
    // this means we will "single-step" through the
    // code, faulting on each instruction.
    brkpt_mismatch_set(pc);
    
    // 4. jump back to the code that faulted.
    // <switchto> loads all the registers <r> and
    // jumps to the pc.  you will write this code
    // next lab.
    switchto(regs);
}

// single system call: just exit by resuming the original 
// scheduler "thread"
static int syscall_handler(regs_t *r) {
    // verify we came from user-level.
    assert(mode_get(r->regs[REGS_CPSR]) == USER_MODE);

    // system call number passed in <r0>, arguments in
    // <r1>,<r2>,<r3>
    uint32_t sysno = r->regs[0];
    uint32_t arg0 = r->regs[1];
    uint32_t pc = r->regs[15];
    switch(sysno) {
    case SS_SYS_EXIT:
        trace("nop_1: exited with syscall=%d: arg0=%d, pc=%x\n", 
            sysno,arg0, pc);

        // ugly: we write out the final registers to a place
        // the caller can get them.
        exit_regs = *r;   

        // done: switch back to the "scheduler" thread.
        switchto(&scheduler_regs);
        not_reached();

    // example: print a character.
    case SS_SYS_PUTC:
        output("%c", r->regs[1]);
        break;
    default:
        panic("illegal system call number: %d\n", sysno);
    }

    // we could return, but show we can use <switchto>
    switchto(r);
    not_reached();
}

// helper routine: make a user-level <cpsr> based on the 
// current <cpsr> value: 
//  - change mode and clear some non-determistic bits.
static uint32_t cpsr_to_user(void) {
    // inherit whatever configured with.
    uint32_t cpsr = cpsr_get();

    //  1. set to user mode,
    cpsr = bits_clr(cpsr, 0, 4) | USER_MODE;
    // 2. clear carry flags (non-determ values)
    cpsr = bits_clr(cpsr, 28, 31);
    // 3. enable interrupts (doesn't matter here).
    cpsr = bit_clr(cpsr, 7);

    // sanity check we didn't mess mode up.
    assert(mode_get(cpsr) == USER_MODE);
    return cpsr;
}


// complete example for how to run single stepping on a simple routine.
// <nop_1>
void notmain(void) {
    //********************************************************
    // 1. Basic initialization
    kmalloc_init_mb(1);

    // install "full register" fault handlers that save
    // and pass in all the registers (16 general purpose
    // and the cpsr).
    full_except_install(0);
    // register a prefetch abort handler: debug exceptions are
    // a subset of these. <single_step_handler> (above)
    full_except_set_prefetch(single_step_handler);
    // register a system call handler (<syscall_handler> defined 
    // above).
    full_except_set_syscall(syscall_handler);

    //******************************************************
    // 2. create an initial register block <nop1_regs> used to 
    //    run the <nop_1> routine (<nop_1> is implemented in 
    //    <single-step-start.S>)

    // the routine we will run.
    void nop_1(void);

    // single-stepping only works on user-level code, so we 
    // need to define a user-level CPSR. (see <cpsr_to_user> 
    // above). 
    uint32_t cpsr = cpsr_to_user();

    // where to start running
    uint32_t initial_pc = (uint32_t)nop_1; 

    // <regs_t>: 17 uint32_t entries (see: <switchto.h>
    //  - 16 general purpose regs r0..r15 (r0 at r.regs[0], 
    //    r1 at r.regs[1], etc).
    //  - the cpsr at r.regs[16].
    regs_t nop1_regs = { 
        .regs[REGS_PC] = initial_pc,
        .regs[REGS_R0] = 0,     // <nop_1> doesnt take any argument
        .regs[REGS_SP] = 0,     // <nop_1> doesnt take any stack
        .regs[REGS_CPSR] = cpsr,// the <cpsr> to use.

        // where to jump to if the code returns.
        // this is just like the hack you did for rpi threads :)
        // <sys_equiv_exit> is defined in <single-step-start.S>
        .regs[REGS_LR] = (uint32_t)exit_trampoline,
    };

    //******************************************************
    // 3. turn on mismatching: we are at privileged mode so won't get 
    // a fault.
    brkpt_mismatch_start();

    // setup so we mismatch on any instruction address by setting
    // mismatch pc=0, which will always mismatch (assuming don't 
    // jump to address 0!
    brkpt_mismatch_set(0);

    //******************************************************
    // 3. switch to it and run!
    // 
    // <scheduler_regs> save our current state so we can resume.  
    // should remind you of lab 5: is essentially the same method
    // as you used in your rpi-thread.c to start the threads
    // in <rpi_thread_start>

    trace("\tcheck the faulting pc's against <0-nop-example.list>:\n");
    trace_nofn("\t00008038 <nop_1>:\n");
    trace_nofn("expected:\t8038:   e320f000    nop {0}\n");
    trace_nofn("expected:\t803c:   e3a00002    mov r0, #2\n");
    trace_nofn("expected:\t8040:   ef000001    svc 0x00000001\n");

    trace("PRE: about to single step <nop_1>\n");

    // 4. save current regs in <scheduler_regs> and load <nop1_regs>.
    // given how we initialized <nop1_regs> in step 2 above
    // this will cause us to start running at <USER_MODE> with 
    // <pc>=nop_1 (0x8038) with all registers set to 0.
    switchto_cswitch(&scheduler_regs, &nop1_regs);

    // note: nop_1 ends with an exit system call that has its
    // exit code in register <r1>: print it.
    trace_nofn("POST: done single-stepping nop_1\n");
    trace_nofn("\tn_inst=%d: non-zero regs= {\n", n_inst);
    for(int i = 0; i < 16; i++) {
        let v = exit_regs.regs[i];
        if(v)
            trace_nofn("\t\tr%d = %x\n", i, v);
    }
    trace_nofn("\t\tcpsr=%x\n", exit_regs.regs[REGS_CPSR]);
    trace_nofn("\t}\n");

    // done mismatching.
    brkpt_mismatch_stop();
}
