#include "rpi-asm.h"
    
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ get USER banked regs by switching to SYSTEM mode and back.
@ if we switched to USER, we would need to use a system call to 
@ get back.

@ get the USER sp by switching to SYS_MODE 
@ and back.
MK_FN(cps_user_sp_get)
    cps #SYS_MODE
    prefetch_flush(r1);

    @ USER And SYSTEM share the same sp/lr/cpsr
    mov r0, sp

    cps #SUPER_MODE
    prefetch_flush(r1);

    bx lr

@ set the USER sp
@   - as above: use <cpsr> to switch to SYSTEM (SYS_MODE) and back.
MK_FN(cps_user_sp_set)
    cps #SYS_MODE
    prefetch_flush(r1);

    @ USER And SYSTEM share the same sp/lr/cpsr
    mov sp, r0

    cps #SUPER_MODE
    prefetch_flush(r1);

    bx lr

@ get the USER lr 
@   - as above: use <cpsr> to switch to SYSTEM (SYS_MODE) and back.
MK_FN(cps_user_lr_get)
    cps #SYS_MODE
    prefetch_flush(r1);

    @ USER And SYSTEM share the same sp/lr/cpsr
    mov r0, lr

    cps #SUPER_MODE
    prefetch_flush(r1);

    bx lr

@ set USER lr 
@   - as above: use <cpsr> -> SYSTEM -> back.
MK_FN(cps_user_lr_set)
    cps #SYS_MODE
    prefetch_flush(r1);

    @ USER And SYSTEM share the same sp/lr/cpsr
    mov lr, r0

    cps #SUPER_MODE
    prefetch_flush(r1);

    bx lr

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ get USER banked regs by using ldm/stm instructions with the ^
@ modifier.

@ address to a uint32_t word is passed in r0
MK_FN(mem_user_sp_get)
    @ dereference the address held in <r0>
    @ and write the 32-bit result into user
    @ mode <sp> register.
    stm r0, {sp}^
    bx lr

@ address to a uint32_t word is passed in r0
MK_FN(mem_user_sp_set)
    ldm r0, {sp}^
    bx lr

@ address to a uint32_t word is passed in r0
MK_FN(mem_user_lr_get)
    stm r0, {lr}^
    bx lr

@ address to a uint32_t word is passed in r0
MK_FN(mem_user_lr_set)
    ldm r0, {lr}^
    bx lr

@ address to an array with two uint32_t words 
@ is passed in r0
MK_FN(mem_user_sp_lr_get)
    stm   r0, {sp,lr}^
    bx lr

@ address to an array with two uint32_t words 
@ is passed in r0
MK_FN(mem_user_sp_lr_set)
    ldm   r0, {sp,lr}^
    bx lr

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ switch to a given privileged mode to get banked sp,lr
@
@ void mode_get_lr_sp_asm(mode, &sp, &lr)
@    - mode in r0
@    - address to hold sp in r1
@    - address to hold lr in r2
MK_FN(mode_get_lr_sp_asm)
    push {lr}
    msr cpsr_c, r0
    prefetch_flush(r0); @ when i use r0 to perform the flush, it works

    @ USER And SYSTEM share the same sp/lr/cpsr
    str sp, [r1]
    str lr, [r2]

    cps #SUPER_MODE
    prefetch_flush(r0); @ when i use r0 to perform the flush, it works

    pop {lr}
    bx lr

MK_FN(mode_set_lr_sp_asm)
    msr cpsr_c, r0
    prefetch_flush(r1);

    @ USER And SYSTEM share the same sp/lr/cpsr
    mov sp, r1
    mov lr, r2


    cps #SUPER_MODE
    prefetch_flush(r1);

    bx lr
