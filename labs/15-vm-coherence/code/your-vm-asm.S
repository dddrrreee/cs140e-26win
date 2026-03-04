@ Must carefully read B2 of the armv6 manual.  You cannot rely on "well it
@ worked on my test case": mistakes cause stale data, which may or may not
@ show up in your test case (likely not) despite being wildly broken.
@
@ Note: Rd is allegedly a read-only parameter to these instructions, but
@ the manual says SBZ (should be zero).  I think (and the linux source seems
@ to think) that this means we have to explicitly set it to 0.  Not setting
@ Rd=0 is an easy mistake to make.  Perhaps is worth building in (Linux does
@ not, but) --- the cost of an extra mov is negligible given our other 
@ overheads.
@
@ Alot of people put these instructions in inline assembly, wrapped up by 
@ function calls (e.g., cs107e's MMU code).  This practice is dangerous. 
@ For example, if you are flushing caches, you cannot be sure that the 
@ function return, etc does not then reload them, esp w.r.t. the BTB.  I 
@ think the only safe approach is to put directly in assembly so you are 
@ guaranteed no branch, indirect jump, load/store happens in between.
@
@ A lot of MMU code out there appears pretty broken b/c they don't do enough
@ flushing operations and/or rely on gcc code generation to not do the above.

#include "rpi-asm.h"
#include "armv6-coprocessor-asm.h"


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ many page numbers are in 
@    armv6-coprocessor-asm.h
@ and the other armv6-*.h files

// just read the cache type: use C code to figure out if it is unified
// b6-13, b6-14
MK_FN(get_cache_type)
    asm_not_implemented()
    bx lr

// void reset(void)
// clear the dcache, icache, tlbs
MK_FN(mmu_reset) // **
    @ b staff_mmu_reset

    // B2-22 recipe
    
    mov r2, #0  @ clear r2
    INV_DCACHE(r2)
    DSB(r2) // TODO figure out if this is important
    INV_ICACHE(r2)
    INV_TLB(r2)
    DSB(r2)
    PREFETCH_FLUSH(r2)

    bx lr

// void cp15_domain_ctrl_wr(uint32_t dom_reg);
// NOTE:
//   - definitely need to do prefetch flush 
//   - I don't *think* we need the full cp15 sync: flush_btb, 
//     dsb, prefetch flush.
MK_FN(cp15_domain_ctrl_wr)
    @ b staff_cp15_domain_ctrl_wr
    MCR p15, 0, r0, c3, c0, 0
    mov r2, #0
    PREFETCH_FLUSH(r2)
    @ asm_not_implemented()
    bx lr

// void cp15_set_procid_ttbr0(uint32_t proc_and_asid, fld_t *pt);
// sequence from b2-25
MK_FN(cp15_set_procid_ttbr0) // **

    // 3-129 DSB before ContextID
    mov r2, #0
    DSB(r2)

    // B2-25 recipe
    ASID_SET(r2)
    FLUSH_BTB(r2) // B2-24 (B2.7.5) after ASID or TTBR
    PREFETCH_FLUSH(r2)
    
    // B4-41 Clear TTBR1 and TTBRC
    TTBR0_SET(r1)
    TTBR1_SET(r2)
    TTBR_BASE_CTRL_WR(r2)
    FLUSH_BTB(r2) // B2-24 (B2.7.5) after ASID or TTBR
    PREFETCH_FLUSH(r2)
    
    // Set ContextID Register with ASID_SET
    ASID_SET(r0)
    FLUSH_BTB(r2) // B2-24 (B2.7.5) after ASID or TTBR
    PREFETCH_FLUSH(r2) // B2-24 for prefetch after CP15

    @ asm_not_implemented()
    bx lr

// void mmu_disable_set_asm(cp15_ctrl_reg1_t c);
MK_FN(mmu_disable_set_asm) // **
    @ b staff_mmu_disable_set_asm

    @ note: this piece of code is mapped into the same place in both
    @ MMU-enabled and MMU-disabled so BTB and Prefetch entries work identically
    @ to both MMU-enable and disable.   Without this requirement I believe is
    @ very hard to get right (perhaps impossible?) since we cannot flush
    @ the BTB and prefetch and switch MMU on/off atomically.  thus, would
    @ flush, but instructions could get fetched before we could disable,
    @ defeating the purpose.

    // In README
    mov r2, #0
    CLEAN_INV_DCACHE(r2)
    DSB(r2) // Flushing bits to memory before clear

    // 1. Clear bit 2 to 0 to disable data cache
    // 6-9 disable dcache and disable mmu simultaneously
    @ bic r0, r0, #0x4 // Assume already set
    MCR p15, 0, r0, c1, c0, 0
    PREFETCH_FLUSH(r2) // B2-24 for prefetch after CP15 to make sure not prefetching early

    // After MMU enable 
    INV_ICACHE(r2)
    FLUSH_BTB(r2) // B2-24 (B2.7.5)
    DSB(r2) // TODO: figure out - completion of issued
    PREFETCH_FLUSH(r2) // B2-24 for prefetch after CP15 - visibility of completed

    @ asm_not_implemented()
    bx lr

// void mmu_enable_set_asm(cp15_ctrl_reg1_t c);
MK_FN(mmu_enable_set_asm) // **
    @ b staff_mmu_enable_set_asm

    mov r2, #0

    // 6-9 - Disabling ICache before MMU
    MRC p15, 0, r1, c1, c0, 0
    bic r1, r1, #0x800 
    MCR p15, 0, r1, c1, c0, 0

    // To make sure icache disable is visible (completed and visible to observers)
    PREFETCH_FLUSH(r2) // B2-24 for prefetch after CP15 to make sure not prefetching early

    // 6-9 - Invalidating ICache before MMU
    INV_ICACHE(r2)
    DSB(r2) // B2-22

    // 6-9 - Enabling ICache before MMU
    @ orr r0, r0, #0x800 // Assume already set
    MCR p15, 0, r0, c1, c0, 0
    PREFETCH_FLUSH(r2) // B2-24 for prefetch after CP15

    FLUSH_BTB(r2) // B2-24 (B2.7.5)
    PREFETCH_FLUSH(r2) // B2-24 for prefetch after CP15

    @ asm_not_implemented()
    bx lr

MK_FN(mmu_sync_pte_mods) // **
    @ b staff_mmu_sync_pte_mods
    
    // B2-23 recipe
    mov r2, #0
    CLEAN_INV_DCACHE(r2) // nuking whole cache instead of line
    INV_ICACHE(r2) // ** WHY DOES THIS WORK???? (README) - Moving up here to make sure 
    DSB(r2)

    INV_TLB(r2)

    // Same DSB, PREFETCH on B2-24
    FLUSH_BTB(r2)
    DSB(r2)
    PREFETCH_FLUSH(r2)

    @ asm_not_implemented()
    bx lr

// **
MK_FN(cp15_ctrl_reg1_rd) // **
    MRC p15, 0, r0, c1, c0, 0 // Don't need prefetch because it is a get
    bx lr

// ** CHANGE FOR AUTOGRADER?
MK_FN(cp15_ctrl_reg1_wr) // **
    MCR p15, 0, r0, c1, c0, 0

    // B2-24 for prefetch after CP15
    mov r2, #0
    PREFETCH_FLUSH(r2)
    bx lr
