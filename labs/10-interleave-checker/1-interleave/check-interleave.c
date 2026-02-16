// engler: starter code for simple A() B() interleaving checker.
// shows 
//   1. how to call the routines.
//   2. how to run code in single step mode.
//
// you'll have to rewrite some of the code to do the actual switching
// of A->B.
#include "check-interleave.h"
#include "full-except.h"
#include "pi-sys-lock.h"

// used to communicate with the breakpoint handler.
static volatile checker_t *checker = 0;

int brk_verbose_p = 0;

static void A_terminated(uint32_t ret);


// invoked from user level: 
//   syscall(sysnum, arg0, arg1, ...)
//
//   - sysnum number is in r->reg[0];
//   - arg0 of the call is in r->reg[1];
//   - arg1 of the call is in r->reg[2];
//   - arg2 of the call is in r->reg[3];
// we don't handle more arguments.
static int syscall_handler_full(regs_t *r) {
    assert(mode_get(cpsr_get()) == SUPER_MODE);

    // we store the first syscall argument in r1
    uint32_t arg0 = r->regs[1];
    uint32_t sys_num = r->regs[0];

    // pc = address of instruction right after 
    // the system call.
    uint32_t pc = r->regs[15];      

    switch(sys_num) {
    case SYS_RESUME:
            // used to do a context switch from user->privileged.
            switchto((void*)arg0);
            panic("not reached\n");
    case SYS_TRYLOCK:
        panic("not handling yet\n");

    case SYS_TEST:
        printk("running empty syscall with arg=%d\n", arg0);
        return SYS_TEST;
    default:
        panic("illegal system call = %d, pc=%x, arg0=%d!\n", sys_num, pc, arg0);
    }
}

// called on each single step exception: 
// if we have run switch_on_inst_n instructions then:
//  1. run B().
//     - if B() succeeds, then run the rest of A() to completion.  
//     - if B() returns 0, it couldn't complete w current state:
//       resume A() and switch on next instruction.
static void single_step_handler_full(regs_t *r) {
    // assume we only get breakpoint faults.
    if(!brkpt_fault_p())
        panic("impossible: should get no other faults\n");

    // r0 is in r->regs[0], r1 is in r->regs[1], ...
    uint32_t pc = r->regs[15];
    uint32_t n = ++checker->inst_count;


    // TODO: you'll have to add code to do the switching here.
    // output("single-step handler: inst=%d: A:pc=%x\n", n,pc);

    // // If at A_terminated, turn off mismatch
    // if (pc == (uint32_t)A_terminated) {
    //     brk_debug("FINALLY TERMINATED at %d with pc: %x\n", n, pc);
    //     brkpt_mismatch_stop();
    //     switchto(r);
    // }

    if (!checker->switched_p && n > checker->switch_on_inst_n) {
        
        // printk("%d\n", n >= checker->switch_on_inst_n);
        if (checker->B((void*)checker) == 1) {
            checker->nswitches++;
            checker->switched_p = 1;
            checker->switch_addr = pc;

            brkpt_mismatch_stop();
        }
        else {
            checker->skips++;
        }
        // Else just run another instruction

        // brk_debug("\n");
        brk_debug("Stopped at %d with pc: %x\n", n, pc);

        // for (uint32_t i = 0; i < 17; i++) {
        //     brk_debug("Reg %d: %x\n", i, r->regs[i]);
        // }
        // brk_debug("\n");
        
    }

    // recall: the weird way single step works: run the instruction 
    // at address <pc>, by setting up a mismatch fault for any other
    // <pc> value.
    brkpt_mismatch_set(pc);

    // If at A_terminated, turn off mismatch
    if (pc == (uint32_t)A_terminated) {
        brk_debug("FINALLY TERMINATED at %d with pc: %x\n", n, pc);
        brkpt_mismatch_stop();
        switchto(r);
    }

    // switch to back.
    switchto(r);
}

// registers saved when we started: recall similar in <rpi-thread.c>
static regs_t start_regs;

// this is called when A() returns: assumes you are at user level.
// switch back to <start_regs>
static void A_terminated(uint32_t ret) {
    uint32_t cpsr = mode_get(cpsr_get());
    if(cpsr != USER_MODE)
        panic("should be at USER, at <%s> mode\n", mode_str(cpsr));

    // put whatever A() returned into r0 position.
    start_regs.regs[0] = ret;
    sys_switchto(&start_regs);
}

// run routine <c->A()> at user level by making a stack,
// switching into it 
static uint32_t run_A_at_userlevel(checker_t *c) {
    // 1. get current cpsr and change mode to USER_MODE.
    // this will preserve any interrupt flags etc.
    uint32_t cpsr_cur = cpsr_get();
    assert(mode_get(cpsr_cur) == SUPER_MODE);
    uint32_t cpsr_A = mode_set(cpsr_cur, USER_MODE);

    // 2. setup the registers.

    // allocate stack on our main stack.  grows down.
    enum { N=8192 };
    uint64_t stack[N];

    // initialize the thread block for <A>.
    // see <switchto.h>
    regs_t r = { 
        // start running A.
        .regs[REGS_PC] = (uint32_t)c->A,
        // the first argument to A()
        .regs[REGS_R0] = (uint32_t)c,

        // stack pointer register
        .regs[REGS_SP] = (uint32_t)&stack[N],
        // the cpsr to use when running A()
        .regs[REGS_CPSR] = cpsr_A,

        // hack: setup registers so that when A() is done 
        // and returns will jump to <A_terminated> by 
        // setting the LR register to <A_terminated>.
        .regs[REGS_LR] = (uint32_t)A_terminated,
    };

    // 3. turn on mismatching
    brkpt_mismatch_start();

    // 4. context switch to <r> and save current execution
    // state in <start_regs> [similar to <rpi-thread.c>
    // when we started the threads package.
    switchto_cswitch(&start_regs, &r);

    // 5. At this point A() finished execution.   We got 
    // here from A_terminated():switchto(&start_regs)

    // 6. turn off mismatch (single step)
    brkpt_mismatch_stop();

    // 7. return back to the checking loop.
    return start_regs.regs[0];
}

// <c> has pointers to four routines:
//  1. A(), B(): the two routines to check.
//  2. init(): initialize A() B() state.
//  3. check(): called after A()B() and returns 1 if checks out.
int check(checker_t *c) {
    // install exception handlers for 
    // (1) system calls
    // (2) prefetch abort (single stepping exception is a type
    //     of prefetch fault)
    // 
    // install is idempotent if already there.
    full_except_install(0);
    full_except_set_syscall(syscall_handler_full);
    full_except_set_prefetch(single_step_handler_full);

    //******************************************************************
    // this is what you build: check that A(),B() code works
    // with one context switch for each possible instruction in
    // A()
    checker = c;

    c->interleaving_p = 1;
    for(int i = 0; ; i++) {

        c->switched_p = 0;
        c->init(c);
        c->switch_on_inst_n = i;
        c->inst_count = 0;
        run_A_at_userlevel(c);

        brk_debug("\n\n");

        // If it ran all the way through, it is done, and don't count as trial
        if (!c->switched_p) {
            break;
        }

        // 
        c->ntrials++;

        if (!c->check(c)) {
            output("ERROR: check failed when switched on address [%x] instructions\n", c->switch_addr);
            c->nerrors++;
        }

    }

    if (c->nerrors > 0) {
        return 0;
    }

    return 1;
}
