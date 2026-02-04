#include "rpi-asm.h"

@ void arm_vector_set(void *base);
@   - set the exception vector base to `base`
MK_FN(arm_vector_set)
    mcr p15, 0, r0, c12, c0, 0
    bx  lr

@ void arm_vector_get(void *base);
@   - get the exception vector base
MK_FN(arm_vector_get)
    mrc p15, 0, r0, c12, c0, 0
    bx  lr