#ifndef __BREAKPOINT_H__
#define __BREAKPOINT_H__

// these will be very close or identical to your 
// <mini-step.c> implementations.  we have renamed
// them b/c there was some variations in how people 
// built there's b/c the README was unclear and the 
// testing was weak so this is a useful sanity check.

// disable mismatch (RMW the enable bit in bcr0)
void brkpt_mismatch_stop(void);
void staff_brkpt_mismatch_stop(void);

// enable mismatch: 
//   - turn on cp14 if need be.
//   - set the mismatch addr in bvr0=0 and 
//     set all the bits in bcr0 (including enable)
//     so will start mismatching.
void brkpt_mismatch_start(void);
void staff_brkpt_mismatch_start(void);

// set mismatch on <addr>:
//  - set bvr0=<addr>
//  - set bcr0 (including enable). 
void brkpt_mismatch_set(uint32_t addr);
void staff_brkpt_mismatch_set(uint32_t addr);

// is it a breakpoint fault?
//   1. was a debug instruction fault (use ifsr)
//   2. breakpoint occured (13-11: use dscr)
int brkpt_fault_p(void);
int staff_brkpt_fault_p(void);

//**************************************************
// trivial match interface.

void brkpt_match_init(void);
void staff_brkpt_match_init(void);

// get the current match (if any).
uint32_t brkpt_match_get(void);
uint32_t staff_brkpt_match_get(void);

// was the current fault a match fault?
int brkpt_match_fault_p(void);
int staff_brkpt_match_fault_p(void);

// set a match breakpoint on <addr>,  use
// bcr1 and bvr1
void brkpt_match_set(uint32_t addr);
void staff_brkpt_match_set(uint32_t addr);

// turn off matching (disable bcr1)
void brkpt_match_stop(void);
void staff_brkpt_match_stop(void);

#endif
