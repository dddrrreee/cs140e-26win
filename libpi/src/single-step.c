#include "single-step.h"
#include "single-step-syscalls.h"
#include "breakpoint.h"
#include "full-except.h"
#include "fast-hash32.h"

#undef trace

static int verbose_p = 1;
static int log_gprof = 0;
static uint32_t check_addr = 0;
static int addr_last_val = 0;

#define trace(msg...) do { if(verbose_p) trace_nofn(msg); } while(0)

void single_step_verbose(int v_p) {
    verbose_p = v_p;
}

void single_step_gprof(int enable) {
    log_gprof = enable;
}

void single_step_check_bad_addr(int addr) {
    check_addr = addr;
}

static const char *ss_cur_fn = "none";
// count how many instructions.
static volatile unsigned n_inst;

// initial registers: what we <switchto> to resume.
static regs_t scheduler_regs;
// hack to record the registers at exit for printing.
static regs_t exit_regs;

// single-step (mismatch) breakpoint handler:
void single_step_handler(regs_t *regs) {
    if(!brkpt_fault_p())
        panic("impossible: should get no other faults\n");
    assert(mode_get(regs->regs[REGS_CPSR]) == USER_MODE);

    // print: inst/fault pc/machine code at pc.
    uint32_t pc = regs->regs[15];
    n_inst++;
    trace("fault:\t%X:\t%X  @ %d\n", pc, GET32(pc), n_inst);
    // trace("adding to gprof...\n");

    // ** ADDING EXTENSIONY THINGS
    if (log_gprof)
        gprof_inc(pc);

    if (check_addr != 0) {
        // trace("Checking address %x: %x \n", check_addr, GET32(check_addr));
        if (GET32(check_addr) != addr_last_val) {
            printk("\n");
            trace("%x not equal to %x\n", GET32(check_addr), addr_last_val);
            panic("WROTE TO ILLEGAL ADDRESS:\t%X  @ %d\n", pc, n_inst);
        }
    }

    // handle uart race condition
    while(!uart_can_put8())
        ;

    // run the faulting instruction, mismatch everything else.
    brkpt_mismatch_set(pc);
    
    // jump back to the code that faulted.
    switchto(regs);
}

// single system call
int syscall_handler(regs_t *r) {
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

static void print_regs(regs_t regs) {
    trace("\tn_inst=%d: non-zero regs= {\n", n_inst);
    for(int i = 0; i < 16; i++) {
        let v = regs.regs[i];
        if(v)
            trace("\t\tr%d = %x\n", i, v);
    }
}
// run function <fp> in single stepping mode.
regs_t single_step_fn(
    const char *fn_name, 
    void (*fp)(), 
    uint32_t arg,
    void *stack,
    uint32_t nbytes) 
{
    regs_t regs = regs_init(fp, arg, stack, nbytes);
    ss_cur_fn = fn_name;

    // ** Sets initial value for the address to check
    if (check_addr != 0) {
        trace("Noting initial value of %x as %x\n", check_addr, GET32(check_addr));
        addr_last_val = GET32(check_addr);
    }

    trace("PRE: about to single step <%s>\n", fn_name);
    
    single_step_on();
    switchto_cswitch(&scheduler_regs, &regs);
    single_step_off();
    trace("POST: done single-stepping %s\n", fn_name);

    /* Comparing registers here */
    for(int i = 0; i < 16; i++) {
        uint32_t old = regs.regs[i];
        uint32_t new = exit_regs.regs[i];

        if (old != new) {
            trace("\t\tRegister r%d changed from %x to %x\n", i, old, new);
        }
    }

    /* Creating hash */
    exit_regs.reg_hash = fast_hash32(exit_regs.regs, sizeof(exit_regs.regs)); // should be 17*4 size
    trace("\t\tHash: %u\n", exit_regs.reg_hash);
    
    trace("\t\tcpsr=%x\n", exit_regs.regs[REGS_CPSR]);
    trace("\t}\n");

    

    return exit_regs;
}
