#include "rpi.h"
#include "rpi-interrupts.h"
#include "asm-helpers.h"
#include "bit-support.h"
#include "breakpoint.h"

// make sure that cp14 (DSCR) is enabled (p13-7)
void brkpt_match_init(void) {
    staff_brkpt_match_init();
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
    staff_brkpt_mismatch_stop();
}

// was this a breakpoint fault? (either mismatch or match)
// check IFSR bits (p 3-66) to see it was a debug fault.
// check DSCR bits (13-11) to see if it was a breakpoint
int brkpt_fault_p(void) {
    return staff_brkpt_fault_p();
}
