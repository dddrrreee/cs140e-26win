#include "rpi-asm.h"

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ part 1: used by <2-simple-save-test.c>
@

@ terminal syscall: print the registers we have.
syscall_reg_save:

    @ asm_todo("save all 17 registers onto stack: pass pointer to syscall_handler");
    @ For last 2 registers
    sub sp, sp, #8 

    @ 0-14 (in for loop)
    stmia sp, {r0-r14}^

    @ Link register of SVR
    @ stia sp, [sp, #(15*4)]
    str lr, [sp, #(15*4)]

    @ User CPSR pushed into SPSR during syscall exception
    mrs r1, spsr
    str r1, [sp, #(16*4)]

    mov r0, sp
    b syscall_handler
    asm_not_reached();

@ use rfe to load <pc> and <cpsr> from the 2 entry
@ array passed in <r0>.
MK_FN(rfe_asm)
    mov sp, r0
    rfe sp
    asm_not_reached();


@ for test 1.
.align 5; 
.globl syscall_reg_save_vec
syscall_reg_save_vec:
    b unhandled_reset
    b unhandled_undefined_instruction
    b syscall_reg_save
    b unhandled_prefetch_abort
    b unhandled_data_abort
    b unhandled_interrupt
    asm_not_reached();

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ used by the rest of tests.

@ don't forget to fix lr!
prefetch_reg_save:
    asm_todo("save 17 registers onto stack: pass pointer to prefetc_abort_handler");
    asm_not_reached();

.align 5; 
.globl simple_reg_save_restore_vec
simple_reg_save_restore_vec:
    b unhandled_reset
    b unhandled_undefined_instruction
    b unhandled_swi
    b prefetch_reg_save
    b unhandled_data_abort
    b unhandled_interrupt
    asm_not_reached();

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ weak symbols used not all tests define all handlers.

@ so we can compile if they don't provide
.weak syscall_handler
MK_FN(syscall_handler)
    asm_not_reached();

@ so we can compile if they don't provide
.weak prefetch_abort_handler
MK_FN(prefetch_abort_handler)
    asm_not_reached();
