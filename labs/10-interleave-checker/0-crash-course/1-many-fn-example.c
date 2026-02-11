// we modify the <0-nop-example.c> example, stripping out comments
// so its more succinct and wrapping up the diffent routines into
// a <single_step_fn> routine that will run a function with an 
// argument in single step mode.
#include "rpi.h"
#include "breakpoint.h"
#include "full-except.h"
#include "cpsr-util.h"
#include "single-step-syscalls.h"

#undef trace

static int verbose_p = 1;
#define trace(msg...) do { if(verbose_p) trace_nofn(msg); } while(0)

void single_step_verbose(int v_p) {
    verbose_p = v_p;
}

static const char *ss_cur_fn = "none";
// count how many instructions.
static volatile unsigned n_inst;

// initial registers: what we <switchto> to resume.
static regs_t scheduler_regs;
// hack to record the registers at exit for printing.
static regs_t exit_regs;

// single-step (mismatch) breakpoint handler:
static void single_step_handler(regs_t *regs) {
    if(!brkpt_fault_p())
        panic("impossible: should get no other faults\n");
    assert(mode_get(regs->regs[REGS_CPSR]) == USER_MODE);

    // print: inst/fault pc/machine code at pc.
    uint32_t pc = regs->regs[15];
    n_inst++;
    trace("fault:\t%X:\t%X  @ %d\n", pc, GET32(pc), n_inst);

    // handle uart race condition
    while(!uart_can_put8())
        ;

    // run the faulting instruction, mismatch everything else.
    brkpt_mismatch_set(pc);
    
    // jump back to the code that faulted.
    switchto(regs);
}

// single system call
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
        trace("%s: exited with syscall=%d: arg0=%d, pc=%x\n", 
            ss_cur_fn, sysno,arg0, pc);
        exit_regs = *r;   
        switchto(&scheduler_regs);
        not_reached();
    // example: print a character.
    case SS_SYS_PUTC:
        output("%c", r->regs[1]);
        break;
    default:
        panic("illegal system call number: %d\n", sysno);
    }
    switchto(r);
    not_reached();
}

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

void single_step_on(void) {
    static int init_p = 0;

    // one-time initialization.
    if(!init_p) {
        init_p = 1;
        full_except_install(0);
        full_except_set_prefetch(single_step_handler);
        full_except_set_syscall(syscall_handler);
    }
    brkpt_mismatch_start();

    // setup so we mismatch on any instruction address
    brkpt_mismatch_set(0);
}

void single_step_off(void) {
    brkpt_mismatch_stop();
}

// run function <fp> in single stepping mode.
static regs_t single_step_fn(
    const char *fn_name, 
    void (*fp)(), 
    uint32_t arg,
    void *stack,
    uint32_t nbytes) 
{
    regs_t regs = regs_init(fp, arg, stack, nbytes);
    ss_cur_fn = fn_name;

    trace("PRE: about to single step <%s>\n", fn_name);
    single_step_on();
    switchto_cswitch(&scheduler_regs, &regs);
    single_step_off();
    trace("POST: done single-stepping %s\n", fn_name);

    trace("\tn_inst=%d: non-zero regs= {\n", n_inst);
    for(int i = 0; i < 16; i++) {
        let v = exit_regs.regs[i];
        if(v)
            trace("\t\tr%d = %x\n", i, v);
    }
    trace("\t\tcpsr=%x\n", exit_regs.regs[REGS_CPSR]);
    trace("\t}\n");

    return exit_regs;
}

// complete example for how to run single stepping on a simple routine.
void notmain(void) {
    kmalloc_init_mb(1);

    // make a single stack at the same address for everyone.
    enum { stack_size = 64 * 1024 };
    void *stack = kmalloc(stack_size);

    // should match exactly.
    trace("******************<nop_1>******************************\n");
    trace("\tcheck the faulting pc's against <0-nop-example.list>:\n");
    trace("\t00008038 <nop_1>:\n");
    trace("expected:\t8038:   e320f000    nop {0}\n");
    trace("expected:\t803c:   e3a00002    mov r0, #2\n");
    trace("expected:\t8040:   ef000001    svc 0x00000001\n");

    regs_t r = {};
    void nop_1(void);
    single_step_fn("nop_1", nop_1, 0, 0, 0);

    trace("******************<nop_10>******************************\n");
    void nop_10(void);
    r = single_step_fn("nop_10", nop_10, 0, 0, 0);

    // this routine used a trampoline: check that the final pc
    // is correct.
    extern uint32_t exit_tramp_pc[]; // see <single-step-start.S>
    trace("expect: pc should be %x!\n", exit_tramp_pc);
    void *exit_pc = (void *)r.regs[15];
    if(exit_pc != exit_tramp_pc)
        panic("final pc should be = %x, is %x!\n", exit_tramp_pc, exit_pc);
    trace("success: exit pc=%x!\n", exit_pc);

    trace("******************<mov_ident>******************************\n");
    void mov_ident(void);
    single_step_fn("mov_ident", mov_ident, 0, 0, 0);

    // same as <nop_10> this routine used a trampoline: check that 
    // the final pc is correct.
    exit_pc = (void *)r.regs[15];
    if(exit_pc != exit_tramp_pc)
        panic("final pc should be = %x, is %x!\n", exit_tramp_pc, exit_pc);
    trace("success: exit pc=%x!\n", exit_pc);

    trace("******************<hello>******************************\n");
    trace("about to print hello world single-step, w/ yapping:\n");
    void hello_asm(void);
    n_inst = 0;
        single_step_fn("hello_asm", hello_asm, 0, stack, stack_size);
    trace("ran %d instructions\n", n_inst);

    trace("******************<hello>******************************\n");
    trace("about to print hello world single-step, no yapping:\n");
    verbose_p = 0;
    void hello_asm(void);
    n_inst = 0;
        single_step_fn("hello_asm", hello_asm, 0, stack, stack_size);
    trace("ran %d instructions\n", n_inst);
}
