
#include "rpi.h"
#include "../pinned-vm.h"
#include "../memmap-default.h"
#include "full-except.h"
#include "memmap.h"

#include "cp-macros.h"

#define MB(x) ((x)*1024*1024)
#define dom_heap 3
#define asid_heap 13
#define TRANSLATION_SECTION_FAULT 0b00101
#define DEFAULT_DOM_REG DOM_client << (dom_kern*2) | DOM_client << (dom_heap*2)
#define BX_LR 0xe12fff1e

// used to store address we will write to
static volatile uint32_t heap_addr;

static void prefetch_handler(regs_t *r) {
    uint32_t pc = r->regs[15];

    // // make sure we faulted on the GET32 address.
    // if(pc != PUT32_ADDR)
    //     panic("illegal fault!  expected %x, got %x\n",
    //         PUT32_ADDR, pc);
    // else
    //     trace("SUCCESS!: got a fault on pc=%x\n", pc);

    // uint32_t dfsr = cp15_dfsr_get();
    // uint32_t fault_status = dfsr & 0x7FF;
    
    // if(fault_status != TRANSLATION_SECTION_FAULT)
    //     panic("Fault status wrong! expected %x, got %x\n",
    //         TRANSLATION_SECTION_FAULT, fault_status);
    // else
    //     trace("SUCCESS!: got fault status=%x\n", fault_status);
    // domain_access_ctrl_set(DEFAULT_DOM_REG | DOM_client << (dom_heap*2));

    // done with test.
    trace("Done!\n");

    clean_reboot();
}


void notmain(void) { 
    kmalloc_init_set_start((void*)SEG_HEAP, MB(1));
    assert((uint32_t)__prog_end__ < SEG_CODE + MB(1));
    assert((uint32_t)__code_start__ >= SEG_CODE);

    // setup a data abort handler (just like last lab).
    full_except_install(0);
    full_except_set_prefetch(prefetch_handler);

    unsigned dom_reg = DOM_client << (dom_kern*2) | DOM_client << (dom_heap*2);
    pin_mmu_init(dom_reg);

    assert(!mmu_is_enabled());
    pin_t dev  = pin_mk_global(dom_kern, no_user, MEM_device);
    pin_t kern = pin_mk_global(dom_kern, no_user, MEM_uncached);
    pin_t heap = pin_mk_user(dom_heap, asid_heap, no_user, MEM_uncached); // New domain ID

    staff_mmu_init();
    pin_mmu_sec(0, SEG_CODE, SEG_CODE, kern);
    pin_mmu_sec(1, SEG_HEAP, SEG_HEAP, heap); 
    pin_mmu_sec(2, SEG_STACK, SEG_STACK, kern); 
    pin_mmu_sec(3, SEG_INT_STACK, SEG_INT_STACK, kern); 
    pin_mmu_sec(4, SEG_BCM_0, SEG_BCM_0, dev); 
    pin_mmu_sec(5, SEG_BCM_1, SEG_BCM_1, dev); 
    pin_mmu_sec(6, SEG_BCM_2, SEG_BCM_2, dev); 



    // Heap address to write "bx lr" encoding
    heap_addr = SEG_HEAP;

    PUT32(heap_addr, BX_LR);

    
    // Should fault on something like this (e.g. will fault on 0x802c)
    
    // staff_mmu_enable();
    // assert(staff_mmu_is_enabled());
    BRANCHTO(heap_addr);
    // output("about to write to heap: <%x>\n", illegal_addr);
    // PUT32(illegal_addr, 0);
    // panic("should not reach here\n");
}
