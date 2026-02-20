// engler, 140e'26: simple test to check that you restore 
// user-mode registers correctly: generalize previous
// test to do N runs.
#include "rpi.h"
#include "breakpoint.h"
#include "cpsr-util.h"
#include "switchto.h"
#include "vector-base.h"
#include "pi-random.h"
#include "fast-hash32.h"

// if you don't want a lot of output set <verbose_p> = 0
static int verbose_p = 1;
#define chk_trace(msg...) \
    do { if(verbose_p) trace(msg); } while(0)

// number of iterations to do.
enum { N = 200 };
// current trial we are on.
static volatile int ntrials = 0;
// running hash of all the registers we've seen in the 
// fault handler.
static uint32_t reg_hash = 0;
// running hash of all the registers we've setup.  
// key: should match reg_hash
static uint32_t expected_hash = 0;

// you wrote this.
void switchto_user_regs_asm(uint32_t regs[17]);

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
        chk_trace("regs[%d]=%d\n", i, regs[i]);
    }
    // should be redundant, but check that saved <cpsr> == <spsr>
    let spsr = spsr_get();
    if(regs[16] != spsr)
        panic("spsr saved incorrectly: spsr=%x, saved=%x\n",
            spsr, regs[16]);
    reg_hash = fast_hash_inc32(regs, 17*sizeof regs[0], reg_hash);

    if(reg_hash != expected_hash)
        panic("reg_hash=%x, expected hash=%x\n", reg_hash, expected_hash);
    chk_trace("regs matched!\n");
    return 1;
}


 __attribute__((noreturn))
static void mutate_and_jump(void) {
    // initialize registers to known values.
    // we dup <regs> and <expected_regs> in case
    // the <switchto_user_regs_asm> has a corruption 
    // bug.
    uint32_t regs[17];
    for(int i = 0; i < 15; i++)
        regs[i] = expected_regs[i] = pi_random();
    // pc doesn't matter as long as 4 byte aligned.
    regs[15] = expected_regs[15] = pi_random() << 2;
    regs[16] = expected_regs[16] = USER_MODE;

    expected_hash = fast_hash_inc32(regs, 17*sizeof regs[0], expected_hash);


    // doesn't matter which address as long as it's not 
    // what we mismatch on :)
    enum { impossible_pc = ~0 << 2 };
    assert(regs[15] != impossible_pc);

    // mismatch on an impossible address that isn't null.
    // should trap before running.
    brkpt_mismatch_set(impossible_pc);
    // load and jump to <regs>
    switchto_user_asm((void*)regs); // gross cast sorry
    not_reached();
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
    if(check_regs(regs, expected_regs))
        trace_nofn("trial %d passed!: expected(%x)==reg(%x)\n",
                 ntrials, expected_hash, reg_hash);

    if(ntrials++ < N) {
        mutate_and_jump();
        not_reached();
    } else {
        assert(reg_hash == expected_hash);
        trace("SUCCESS: %d trials!\n", N);
        clean_reboot();
    }
}

void notmain(void) {
    // setup vector see <interrupt-asm.S>
    extern uint32_t simple_reg_save_restore_vec[];
    vector_base_set(simple_reg_save_restore_vec);

    verbose_p = 0;
    pi_random_seed(10);
    brkpt_mismatch_start();
    mutate_and_jump();
    not_reached();
}
