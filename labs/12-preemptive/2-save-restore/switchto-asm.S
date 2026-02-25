#include "rpi-asm.h"

@ Executed in SUPER_MODE from test
MK_FN(switchto_user_asm)
    @ r0 = pointer to 17-entry regs array
    mov sp, r0

    # Take stuff from array into registers (does not change the sp)
    ldm sp, {r0-r14}^

    @ Goes to where lr and cpsr are
    add sp, sp, #(15*4)
    rfe sp
    asm_not_reached();

@ NOTE:  set <cpsr> using the following instructon so GNU
@ assembler will set all the bits.  get mismatches otherwise
@   msr   cpsr_cxsf, <reg>
MK_FN(switchto_priv_asm)
    @ asm_todo("ONE OF THE FUNCTIONS")

    @ ----- Inside FIRST mode -----
    @ put array in sp_first and save current mode
    mov sp, r0
    mrs r3, cpsr
    add r1, r0, #(8*4)

    @ Switch to other mode and get pointer from FIRST mode
    ldr r4, [sp, #(16*4)]
    msr cpsr_c, r4
    prefetch_flush(r0); @ when i use r0 to perform the flush, it works

    @ ----- Inside SECOND mode -----
    ldm r1, {r8-r14}

    @ Switch back
    msr cpsr_c, r3
    prefetch_flush(r0); @ when i use r0 to perform the flush, it works

    @ ----- Inside FIRST mode -----

    @ Set remaining regs
    ldm sp, {r0-r7}

    add sp, sp, #(15*4)
    rfe sp
    @ b staff_switchto_priv_asm
    asm_not_reached();

@ void priv_get_sp_lr_asm(uint32_t mode, uint32_t *sp, uint32_t *lr);
@   - assumes caller is at privileged mode [we could check this]
@   - mode in r0
@   - sp pointer in r1
@   - lr pointer in r2
@
MK_FN(priv_get_sp_lr_asm)
    @ asm_todo("ONE OF THE FUNCTIONS")
    push {lr}

    @ Store previous cpsr and switch to temporary one
    mrs r3, cpsr
    msr cpsr_c, r0
    prefetch_flush(r0); @ when i use r0 to perform the flush, it works

    @ Obtain
    str sp, [r1]
    str lr, [r2]

    msr cpsr_c, r3
    prefetch_flush(r0); @ when i use r0 to perform the flush, it works
    pop {lr}
    bx lr

@ you'll have to write this, but it should mirror your other save
@ routines.
@
@ void cswitchto_priv_asm(regs_t *old, regs_t *next);
@
@ voluntary context switch: save the current registers in <old>
@ and call the right switchto on <next>
MK_FN(cswitchto_priv_asm)
    @ initially just forward to ours.
    @ b staff_cswitchto_priv_asm

    @ to implement:
    @ 1. save all the callee regs (just like in your threads library)
    @    into the 17-entry register array we have been using.  note
    @    these have to be at the right offsets.
    @    e.g.:
    @       str r4,  [r0, #(4*4)]  @ r4
    @       str r5,  [r0, #(5*4)]  @ r5
    @       ... etc ...
    @   don't forget the cpsr!


    @ 2. just call the right switch routine: note, we should
    @    *not* reach this location when switching back!
    bl switchto_priv_asm
    asm_not_reached();

@ you'll have to write this, but it should mirror your other save
@ routines.
@
@ void cswitchto_user_asm(regs_t *old, regs_t *next);
@
@ voluntary context switch: save the current registers in <old>
@ and call the right switchto on <next>
MK_FN(cswitchto_user_asm)
    @ initially just forward to ours.
    @ b staff_cswitchto_user_asm

    asm_todo("save the registers: identical to <switchto_priv_asm>")
    bl switchto_user_asm
    asm_not_reached();
