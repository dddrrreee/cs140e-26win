// we also use the enums in <memmap-default.h> which
// defines:
// - the default segments and <no_user>
//   (identical to <1-test-basic.c>)
// - dom_kern and dom_user:
//   enum { 
//      dom_kern = 1, // domain id for kernel
//      dom_user = 2  // domain id for user
//   };          
//
// we also switch to using the pinned calls.
#include "rpi.h"
#include "full-except.h"
#include "../pt-vm.h"
#include "../mmu.h"
#include "../memmap-default.h"

void notmain(void) { 
    // our standard init.
    kmalloc_init_set_start((void*)SEG_HEAP, MB(1));
    full_except_install(0);

    //****************************************************
    // 1. map kernel memory 
    //  - same as <1-test-basic.c>

    // device memory: kernel domain, no user access, 
    // memory is strongly ordered, not shared.
    pin_t dev  = pin_mk_global(dom_kern, no_user, MEM_device);
    // kernel memory: same, but is only uncached.
    pin_t kern = pin_mk_global(dom_kern, no_user, MEM_uncached);

    // TWO page tables for each 
    vm_pt_t *pt1 = vm_pt_alloc(PT_LEVEL1_N/2);
    vm_pt_t *pt2 = vm_pt_alloc(PT_LEVEL1_N/2);

    vm_mmu_init(~0);
    assert(!mmu_is_enabled());


    // Need to map both to the same parts 
    vm_map_sec(pt1, SEG_CODE, SEG_CODE, kern);
    vm_map_sec(pt1, SEG_HEAP, SEG_HEAP, kern);
    vm_map_sec(pt1, SEG_STACK, SEG_STACK, kern);
    vm_map_sec(pt1, SEG_INT_STACK, SEG_INT_STACK, kern);
    vm_map_sec(pt1, SEG_BCM_0, SEG_BCM_0, dev);
    vm_map_sec(pt1, SEG_BCM_1, SEG_BCM_1, dev);
    vm_map_sec(pt1, SEG_BCM_2, SEG_BCM_2, dev);

    vm_map_sec(pt2, SEG_CODE, SEG_CODE, kern);
    vm_map_sec(pt2, SEG_HEAP, SEG_HEAP, kern);
    vm_map_sec(pt2, SEG_STACK, SEG_STACK, kern);
    vm_map_sec(pt2, SEG_INT_STACK, SEG_INT_STACK, kern);
    vm_map_sec(pt2, SEG_BCM_0, SEG_BCM_0, dev);
    vm_map_sec(pt2, SEG_BCM_1, SEG_BCM_1, dev);
    vm_map_sec(pt2, SEG_BCM_2, SEG_BCM_2, dev);

    //****************************************************
    // 2. Create user entries in TWO different address spaces


    // for today: user entry attributes are:
    //  - non-global
    //  - user dom
    //  - user r/w permissions. 
    // for this test:
    //  - also uncached (like kernel)
    //  - ASID = 1
    enum { ASID1 = 6, ASID2 = 7 }; // Just for ease of reading

    pin_t asid1_usr = pin_mk_user(dom_user, ASID1, user_access, MEM_uncached);
    pin_t asid2_usr = pin_mk_user(dom_user, ASID2, user_access, MEM_uncached);
    
    
    // Non-identity map (keep OUT of kernel space)
    enum { 
        user1_va_addr = MB(6),
        user2_va_addr = MB(7),

        asid1_pa1    = MB(16),
        asid1_pa2    = MB(17),

        asid2_pa1    = MB(18),
        asid2_pa2    = MB(19),
    };

    vm_map_sec(pt1, user1_va_addr, asid1_pa1, asid1_usr);
    vm_map_sec(pt1, user2_va_addr, asid1_pa2, asid1_usr);

    // For ASID2
    vm_map_sec(pt2, user1_va_addr, asid2_pa1, asid2_usr);
    vm_map_sec(pt2, user2_va_addr, asid2_pa2, asid2_usr);

    // Set up addresses before turning vm on.
    PUT32(asid1_pa1, 0x11);
    PUT32(asid1_pa2, 0x12);
    PUT32(asid2_pa1, 0x21);
    PUT32(asid2_pa2, 0x22);

    //****************************************************
    // 3. Turn on MMU and test ASID 1
    //    
    uint32_t translated_pa, res;

    vm_mmu_switch(pt1,0x140e,ASID1);
    vm_mmu_enable();
    if(vm_xlate(&translated_pa, pt2, SEG_ILLEGAL))
        panic("illegal address is mapped??\n");
    
    // Make sure translations exist for both users 1 and 2
    // Loops through 4 because address of 32-bit

    for (uint32_t offset = 0; offset < MB(1); offset += 4) {

        // User 1
        if(!vm_xlate(&translated_pa, pt1, user1_va_addr + offset))
            panic("user address is missing\n");
        else if(translated_pa != asid1_pa1 + offset)
            panic("should be an identity mapping: exp=%x, got %x\n", 
                asid1_pa1 + offset, translated_pa);

        // User 2
        if(!vm_xlate(&translated_pa, pt1, user2_va_addr + offset))
            panic("user address is missing\n");
        else if(translated_pa != asid1_pa2 + offset)
            panic("should be an identity mapping: exp=%x, got %x\n", 
                asid1_pa2 + offset, translated_pa);
    }
    trace("asid1 all offsets from 0-1MB translated correctly\n");

    // Check values we put there
    res = GET32(user1_va_addr);
    trace("asid1 (MMU) = got: %x\n", res);
    assert(res == 0x11);
    res = GET32(user2_va_addr);
    trace("asid1 (MMU) = got: %x\n", res);
    assert(res == 0x12);

    //****************************************************
    // 4. Turn off MMU and test ASID 1
    // 

    vm_mmu_disable();
    assert(!mmu_is_enabled());

    // Checks virtual address and should not be the values
    res = GET32(user1_va_addr);
    trace("asid1 (NO MMU) = got: %x\n", res);
    assert(res != 0x11);
    res = GET32(user2_va_addr);
    trace("asid1 (NO MMU) = got: %x\n", res);
    assert(res != 0x12);

    // Should be the values at their actual physical address
    res = GET32(asid1_pa1);
    trace("asid1 phys_addr (NO MMU) = got: %x\n", res);
    assert(res == 0x11);
    res = GET32(asid1_pa2);
    trace("asid1 phys_addr (NO MMU) = got: %x\n", res);
    assert(res == 0x12);

    //****************************************************
    // 5. Turn on MMU and test ASID 2
    //    

    vm_mmu_switch(pt2,0x546,ASID2);
    vm_mmu_enable();
    if(vm_xlate(&translated_pa, pt1, SEG_ILLEGAL))
        panic("illegal address is mapped??\n");
    
    // Make sure translations exist for both users 1 and 2
    // Loops through 4 because address of 32-bit
    for (uint32_t offset = 0; offset < MB(1); offset += 4) {

        // User 1
        if(!vm_xlate(&translated_pa, pt2, user1_va_addr + offset))
            panic("user address is missing\n");
        else if(translated_pa != asid2_pa1 + offset)
            panic("should be an identity mapping: exp=%x, got %x\n", 
                asid2_pa1 + offset, translated_pa);

        // User 2
        if(!vm_xlate(&translated_pa, pt2, user2_va_addr + offset))
            panic("user address is missing\n");
        else if(translated_pa != asid2_pa2 + offset)
            panic("should be an identity mapping: exp=%x, got %x\n", 
                asid2_pa2 + offset, translated_pa);
    }
    trace("asid2 all offsets from 0-1MB translated correctly\n");

    // Check values we put there
    res = GET32(user1_va_addr);
    trace("asid2 (MMU) = got: %x\n", res);
    assert(res == 0x21);
    res = GET32(user2_va_addr);
    trace("asid2 (MMU) = got: %x\n", res);
    assert(res == 0x22);

    //****************************************************
    // 3. Turn off MMU and test ASID 1
    // 

    vm_mmu_disable();
    assert(!mmu_is_enabled());
    
    // Checks virtual address and should not be the values
    res = GET32(user1_va_addr);
    trace("asid1 (NO MMU) = got: %x\n", res);
    assert(res != 0x21);
    res = GET32(user2_va_addr);
    trace("asid1 (NO MMU) = got: %x\n", res);
    assert(res != 0x22);

    // Should be the values at their actual physical address
    res = GET32(asid2_pa1);
    trace("asid2 phys_addr (NO MMU) = got: %x\n", res);
    assert(res == 0x21);
    res = GET32(asid2_pa2);
    trace("asid2 phys_addr (NO MMU) = got: %x\n", res);
    assert(res == 0x22);

    // make sure can't access illegal: perhaps should have asid?
    if(vm_xlate(&translated_pa, pt1, SEG_ILLEGAL))
        panic("illegal address is mapped??\n");
    
    assert(!mmu_is_enabled());

    trace("MMU is off!\n");
    trace("SUCCESS!\n");
}
