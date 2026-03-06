

.globl fat32_jump_trampoline
fat32_jump_trampoline:
    blx r0      @ call program addr
    bx lr       @ return to caller when program returns