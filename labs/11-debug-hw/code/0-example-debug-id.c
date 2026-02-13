// check that we can get the debug out of cp14.
//
// gives an example of:
//   - how to define the inline assembly using our macros
//   - how to use structures with bitfields (see armv6-debug.h 
//     for the debug_id struct)
//   - crucially: how to use check bitfield offset/size checking macros.
//
// this "test" isn't one: should always just pass.
#include "rpi.h"

// defines useful bit manipulation routines.  you can write your
// own!
#include "bit-support.h"

// see libpi/include: defines useful macros:
// <cp_asm>: uses cpp tricks to define _set and _get versions
// of inline macros.
//   - <cp_asm_get>: make the _get version only.
//   - <cp_asm_set>: make the _set version only.
#include "asm-helpers.h"

// DIDR register defined on page 13-6 of the debug chapter.  
// defines useful information about debug hardware.  pretty
// common to have --- new hardware look for such config registers
// and use for sanity checking.
//
// 13-26: gives the literal assembly instruction.
// this invocation will generate a 
//   <static inline uint32_t cp14_didr_get(void) { ... }>
// routine.
cp_asm_get(cp14_didr, p14, 0, c0, c0, 0)

void notmain(void) {
    // 13-6: get the debug id register value
    uint32_t didr = cp14_didr_get();

    // sanity check it using the values given on 13-7.
    uint32_t
    wrp     = bits_get(didr, 28, 31),
    brp     = bits_get(didr, 24, 27),
    version = bits_get(didr, 16, 19),
    variant = bits_get(didr, 4, 7),
    rev     = bits_get(didr, 0, 4);

    // 13-7: number of watchpoint regs should be 2 (+1 the actual value)
    trace("total watchpoints = %d\n", wrp+1);
    if(wrp + 1 != 2)
        panic("expected 2 watchpoint registers: have %d\n", wrp+1);

    // 13-7: number of breakpoint regs should be 6 (+1 the actual value)
    trace("total breakpoints = %d\n", brp+1);
    if(brp + 1 != 6)
        panic("expected 6 breakpoint registers: have %d\n", brp+1);

    // 13-7: we expected version = 2
    trace("version = %x\n", version);
    if(version != 0x2)
        panic("expected v6.1 [2] have %d\n", version);

    // not sure if this can vary by revision
    trace("variant=[%x]: mine is 0x0 --- yours might differ\n", variant);
    trace("rev=[%x]    : mine is 0x7 --- yours might differ\n", rev);
    trace("SUCCESS: read debug register correctly.\n");
}
