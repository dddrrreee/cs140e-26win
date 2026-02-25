#include "rpi-asm.h"

@ Executed in SUPER_MODE from test
MK_FN(switchto_user_asm)
    @ r0 = pointer to 17-entry regs array
    mov sp, r0
    ldm sp, {r0-r14}^
    add sp, sp, #(15*4)
    rfe sp

    @ ldr r1, [sp, #(16*4)]
    @ msr spsr, r1

    @ Load saved pc into lr_svc (supervisor)
    @ so it can be moved to pc later
    @ ldr lr, [sp, #(15*4)]
    
    # Take stuff from array into registers (does not change the sp)
    ldm sp, {r0-r14}^
    
    @ mov pc,

    @ nop                         @ required after banked reg access
    

    @ Goes to where lr and cpsr are
    add sp, sp, #(15*4)

    rfe sp
    asm_not_reached();

@ NOTE:  set <cpsr> using the following instructon so GNU
@ assembler will set all the bits.  get mismatches otherwise
@   msr   cpsr_cxsf, <reg>
MK_FN(switchto_priv_asm)
    b staff_switchto_priv_asm
    asm_not_reached();

@ void priv_get_sp_lr_asm(uint32_t mode, uint32_t *sp, uint32_t *lr);
@   - assumes caller is at privileged mode [we could check this]
@   - mode in r0
@   - sp pointer in r1
@   - lr pointer in r2
@
MK_FN(priv_get_sp_lr_asm)
    b staff_priv_get_sp_lr_asm
    asm_not_reached();

@ you'll have to write this, but it should mirror your other save
@ routines.
@
@ void cswitchto_priv_asm(regs_t *old, regs_t *next);
@
@ voluntary context switch: save the current registers in <old>
@ and call the right switchto on <next>
MK_FN(cswitchto_priv_asm)
    @ initially just forward to ours.
    b staff_cswitchto_priv_asm

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
    b staff_cswitchto_user_asm

    asm_todo("save the registers: identical to <switchto_priv_asm>")
    bl switchto_user_asm
    asm_not_reached();
