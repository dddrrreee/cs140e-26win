#if 0
#include "rpi.h"
#include "../pt-vm.h"
#include "../memmap-default.h"
#include "full-except.h"
#include "memmap.h"

#include "cp-macros.h"

#define MB(x) ((x)*1024*1024)
#define dom_bad 3
#define asid_heap 13
#define DOMAIN_SECTION_FAULT 0b01001
#define BX_LR 0xe12fff1e

void notmain(void) { 
    kmalloc_init_set_start((void*)SEG_HEAP, MB(1));
    assert((uint32_t)__prog_end__ < SEG_CODE + MB(1));
    assert((uint32_t)__code_start__ >= SEG_CODE);

    assert(!mmu_is_enabled());
    
    // ------------------------------------------------------------
    //               Make process map
    // ------------------------------------------------------------
    procmap_t procmap = procmap_default_mk(dom_kern);
    procmap_push(&procmap, pr_ent_mk(SEG_ILLEGAL, MB(1), MEM_RW, dom_user));
    
    // Make page table kernel
    vm_pt_t* page_table = vm_map_kernel(&procmap, 1);

    uint32_t res, pa, va = SEG_HEAP;
    trace("Trying VA of %x\n", va);

    if(!vm_xlate(&pa, page_table, va))
        panic("user address is missing\n");
    else if(pa != va)
        panic("should be an identity mapping: exp=%x, got %x\n", 
            va, pa);

}
#endif

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

    vm_pt_t *pt = vm_pt_alloc(PT_LEVEL1_N);
    vm_mmu_init(~0);
    assert(!mmu_is_enabled());

    vm_map_sec(pt, SEG_CODE, SEG_CODE, kern);
    vm_map_sec(pt, SEG_HEAP, SEG_HEAP, kern);
    vm_map_sec(pt, SEG_STACK, SEG_STACK, kern);
    vm_map_sec(pt, SEG_INT_STACK, SEG_INT_STACK, kern);
    vm_map_sec(pt, SEG_BCM_0, SEG_BCM_0, dev);
    vm_map_sec(pt, SEG_BCM_1, SEG_BCM_1, dev);
    vm_map_sec(pt, SEG_BCM_2, SEG_BCM_2, dev);

    //****************************************************
    // 2. create a single user entry:


    // for today: user entry attributes are:
    //  - non-global
    //  - user dom
    //  - user r/w permissions. 
    // for this test:
    //  - also uncached (like kernel)
    //  - ASID = 1
    enum { ASID = 1 };
    pin_t usr = pin_mk_user(dom_user, ASID, user_access, MEM_uncached);
    
    // Non-identity map
    enum { 
        user_va_addr = MB(16),
        user_pa_addr = MB(17),
    };
    vm_map_sec(pt, user_va_addr, user_pa_addr, usr);

    // initialize before turning vm on.
    PUT32(user_pa_addr, 0xdeadbeef);

    //****************************************************
    // 3. Turn on MMU
    //    

    trace("about to enable\n");

    // setup address space context before turning on.
    vm_mmu_switch(pt,0x140e,ASID);

    // make sure no helper enabled by mistake
    assert(!mmu_is_enabled());
    vm_mmu_enable();
    assert(mmu_is_enabled());

    //****************************************************
    // 4. Make sure address translates correctly
    //    

    

    // checking that lookup of user address
    // succeeds and of illegal does not.
    uint32_t base_pa = user_pa_addr;
    uint32_t base_va = user_va_addr;
    uint32_t res, pa;

    // Loops through 4 because address of 32-bit
    for (uint32_t offset = 0; offset < MB(1); offset += 4) {
        
        if(!vm_xlate(&pa, pt, base_va + offset))
            panic("user address is missing\n");

        else if(pa != base_pa + offset)
            panic("should be an identity mapping: exp=%x, got %x\n", 
                base_pa + offset, pa);

        printk("pa: %x\tva:%x\n", pa, base_pa);
    }


    // if(!vm_xlate(&pa, pt, va))
    //     panic("user address is missing\n");
    // else if(pa != expected_pa)
    //     panic("should be an identity mapping: exp=%x, got %x\n", 
    //         expected_pa, pa);

    // make sure can't access illegal: perhaps should have asid?
    if(vm_xlate(&pa, pt, SEG_ILLEGAL))
        panic("illegal address is mapped??\n");

    trace("MMU is on and working!\n");
    
    // 4. now turn on the MMU and make sure the expected
    //    entries are there.
    // now r/w user address.
    uint32_t x = GET32(user_va_addr);
    trace("asid 1 = got: %x\n", x);
    assert(x == 0xdeadbeef);
    PUT32(user_va_addr, 1);

    vm_mmu_disable();
    assert(!mmu_is_enabled());

    // check that the write was what we expected.
    trace("MMU is off!\n");
    trace("phys addr1=%x\n", GET32(user_pa_addr));
    assert(GET32(user_pa_addr) == 1);

    trace("SUCCESS!\n");
}
