
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
