// NOTE: you need to implement <todo> and handle all 
// registers from r0-r12
//
// we does some sleazy tricks to mechanically (automatically)
// verify which registers r0-r12 gcc thinks are caller-saved 
// and which are callee-saved.
//
// intuition:
//    we know from the <notes/caller-callee/README.md> that
//    if a routine contains a single gcc's inline assembly 
//    statement that "clobbers" a register (i.e., tells gcc 
//    that the assembly modifies it) that gcc will save and 
//    restore the register iff it's callee-saved and do 
//    nothing (just return) if it's caller-saved.
//
#include "rpi.h"

// write <is_empty>: given a routine address <fp>, return 1 if 
// <fp> does nothing but return.
//  1. figure out what machine instruction should be the first
//     (only) instruction in the routine.
//  2. figure out how to get the first instruction given
//     a function address.
//  3. return 1 if (2) == (1).
//
// we use this to tell if a register <r> is caller saved:
//   1. generate a routine that only clobbers <r>
//   2. if routine is empty, <r> was a caller reg.
//   3. if routine is not empty, <r> was a callee reg.
static inline int is_empty(void (*fp)(void)) {
    // todo("returns 1 if routine does nothing besides return");

    uint32_t instruction = *(uint32_t*)fp;

    // ** If it returns, the next instruction is "bx lr" 1110 00010010 111111111111 0001 1110
    // ** Can find the encoding in the armv6.pdf A4.1.10
    // ** Condition (No Condition, so AL) --> 1110 --> E
    // ** then 00010010 --> 12
    // ** 3x SBO so 1111 1111 1111 --> FFF
    // ** 0001 --> 1
    // ** Rm --> r14 --> D
    // So --> 0xE12FFF1E
    // printk("%x\n", instruction);
    return instruction == 0xE12FFF1E;

    
}

// note: you can do these better with macros.
static void clobber_r0(void) {
    asm volatile("" : : : "r0");
}
static void clobber_r1(void) {
    asm volatile("" : : : "r1");
}
static void clobber_r2(void) {
    asm volatile("" : : : "r2");
}
static void clobber_r3(void) {
    asm volatile("" : : : "r3");
}
static void clobber_r4(void) {
    asm volatile("" : : : "r4");
}
static void clobber_r5(void) {
    asm volatile("" : : : "r5");
}
static void clobber_r6(void) {
    asm volatile("" : : : "r6");
}
static void clobber_r7(void) {
    asm volatile("" : : : "r7");
}
static void clobber_r8(void) {
    asm volatile("" : : : "r8");
}
static void clobber_r9(void) {
    asm volatile("" : : : "r9");
}
static void clobber_r10(void) {
    asm volatile("" : : : "r10");
}
static void clobber_r11(void) {
    asm volatile("" : : : "r11");
}
static void clobber_r12(void) {
    asm volatile("" : : : "r12");
}


// FILL this in with all caller-saved registers.
// these are all the registers you *DO NOT* save during 
// voluntary context switching 
//
// NOTE: we know we have to save r13,r14,r15 so ignore
// them.
void check_cswitch_ignore_regs(void) {
    assert(is_empty(clobber_r0));
    assert(is_empty(clobber_r1));
    assert(is_empty(clobber_r2));
    assert(is_empty(clobber_r3));
    assert(is_empty(clobber_r12));

    // if you reach here it passed.
    trace("CALLER regs passed\n");
}


// put all the regs you *do* save during context switching
// here [callee saved] 
//
// NOTE: we know we have to save r13, r14 (why?) so 
// ignore them.
void check_cswitch_save_regs(void) {
    assert(!is_empty(clobber_r4)); // should NOT be empty
    assert(!is_empty(clobber_r5));
    assert(!is_empty(clobber_r6));
    assert(!is_empty(clobber_r7));
    assert(!is_empty(clobber_r8));
    assert(!is_empty(clobber_r9));
    assert(!is_empty(clobber_r10));
    assert(!is_empty(clobber_r11));
    trace("CALLEE regs passed\n");
}

void notmain() {
    check_cswitch_ignore_regs();
    check_cswitch_save_regs();
    trace("SUCCESS\n");
}
