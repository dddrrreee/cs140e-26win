#ifndef __COPROCESSOR_MACROS_H__
#define __COPROCESSOR_MACROS_H__

#include <stdint.h>
#include "asm-helpers.h"

// Last 2 are the Opcode_2 and CRm
// P14
cp_asm_set(cp14_dscr, p14, 0, c0, c1, 0); // P14 c1
cp_asm_get(cp14_dscr, p14, 0, c0, c1, 0); // P14 c1

cp_asm_set(cp14_wfar, p14, 0, c0, c6, 0); // P14 c6
cp_asm_get(cp14_wfar, p14, 0, c0, c6, 0); // P14 c6

cp_asm_set(cp14_bvr0, p14, 0, c0, c0, 0b100); // P14 c64-69 (using c64)
cp_asm_get(cp14_bvr0, p14, 0, c0, c0, 0b100); // P14 c64-69 (using c64)
cp_asm_set(cp14_bvr1, p14, 0, c0, c1, 0b100); // P14 c64-69 (using c65)
cp_asm_get(cp14_bvr1, p14, 0, c0, c1, 0b100); // P14 c64-69 (using c65)

cp_asm_set(cp14_bcr0, p14, 0, c0, c0, 0b101); // P14 c80-85 (using c80)
cp_asm_get(cp14_bcr0, p14, 0, c0, c0, 0b101); // P14 c80-85 (using c80)
cp_asm_set(cp14_bcr1, p14, 0, c0, c1, 0b101); // P14 c80-85 (using c81)
cp_asm_get(cp14_bcr1, p14, 0, c0, c1, 0b101); // P14 c80-85 (using c81)

cp_asm_set(cp14_wvr, p14, 0, c0, c0, 0b110); // P14 c96-97 (using c96)
cp_asm_get(cp14_wvr, p14, 0, c0, c0, 0b110); // P14 c96-97 (using c96)

cp_asm_set(cp14_wcr, p14, 0, c0, c0, 0b111); // P14 c112-113 (using c112)
cp_asm_get(cp14_wcr, p14, 0, c0, c0, 0b111); // P14 c112-113 (using c112)

// P15
cp_asm_set(cp15_dfsr, p15, 0, c5, c0, 0); // P15 c5
cp_asm_get(cp15_dfsr, p15, 0, c5, c0, 0); // P15 c5

cp_asm_set(cp15_ifsr, p15, 0, c5, c0, 1); // P14 c5 (also?)
cp_asm_get(cp15_ifsr, p15, 0, c5, c0, 1); // P14 c5 (also?)

cp_asm_set(cp15_far, p15, 0, c6, c0, 0); // P15 c6
cp_asm_get(cp15_far, p15, 0, c6, c0, 0); // P15 c6

#endif