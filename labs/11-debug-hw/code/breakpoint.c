#include "rpi.h"
#include "rpi-interrupts.h"
#include "coprocessor-macros.h"
#include "bit-support.h"
#include "breakpoint.h"

// make sure that cp14 (DSCR) is enabled (p13-7)
void brkpt_match_init(void) {
    uint32_t dscr = cp14_dscr_get();
    dscr |= (1 << 15); // The Monitor debug-mode enable bit
    dscr &= ~(1 << 14); // Mode select bit (selects monitor-debug mode)
    cp14_dscr_set(dscr);
}

// set match on <addr>
//
// for simplicity: for matching we use breakpoint 1 
// (bcr1 p13-17 and bvr1 p13-16) so dont't conflict 
// with single-stepping.   
void brkpt_match_set(uint32_t addr) {
    staff_brkpt_match_set(addr);
}

// turn off match faults (clear bcr1)
void brkpt_match_stop(void) {
    uint32_t bcr1 = cp14_bcr1_get();
    bcr1 &= ~1;                 // BCR1[0] enable watchpoint bit cleared
    cp14_bcr1_set(bcr1);

    prefetch_flush();
    staff_brkpt_match_stop();
}

// return the match addr (bvr1)
uint32_t brkpt_match_get(void) {
    return staff_brkpt_match_get();
}


// set mismatch on <addr>
//
// for simplicity: for matching we use breakpoint 0. 
// so set bcr0 and bvr0.
void brkpt_mismatch_set(uint32_t addr) {
    staff_brkpt_mismatch_set(addr);
}

// this will mismatch on the first instruction at user level.
void brkpt_mismatch_start(void) {
    // 1. check DSCR: if not enabled, enable it.
    // 2. set brkpt_mismatch_set(0)
    staff_brkpt_mismatch_start();
}

// turn off mismatching: clear bcr0
void brkpt_mismatch_stop(void) {
    uint32_t bcr0 = cp14_bcr0_get();
    bcr0 &= ~1;                 // BCR0[0] enable watchpoint bit cleared
    cp14_bcr0_set(bcr0);

    prefetch_flush();
    // staff_brkpt_mismatch_stop();
}

// was this a breakpoint fault? (either mismatch or match)
// check IFSR bits (p 3-66) to see it was a debug fault.
// check DSCR bits (13-11) to see if it was a breakpoint
int brkpt_fault_p(void) {
    uint32_t ifsr = cp15_ifsr_get();
    uint32_t dscr = cp14_dscr_get();

    int was_debug_event = (ifsr & 0b1111) == 0b0010;
    int was_brkpt_fault = (dscr >> 2 & 0b1111) == 0b0001;
    return was_debug_event && was_brkpt_fault;
}
