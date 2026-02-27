
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

// used to store the illegal address we will write.
static volatile uint32_t illegal_addr;
static volatile uint32_t GET32_ADDR = (uint32_t)&GET32;

static void data_abort_handler(regs_t *r) {
    uint32_t pc = r->regs[15];

    // make sure we faulted on the GET32 address.
    if(pc != GET32_ADDR)
        panic("illegal fault!  expected %x, got %x\n",
            GET32_ADDR, pc);
    else
        trace("SUCCESS!: got a fault on pc=%x\n", pc);

    // trace("dfsr: %x\n", cp15_dfsr_get());
    // trace("ifsr: %x\n", cp15_ifsr_get());
    uint32_t dfsr = cp15_dfsr_get();
    uint32_t fault_status = 
        ( (dfsr >> 10) & 0b1 )
        | (dfsr & 0b111);
    
    if(fault_status != TRANSLATION_SECTION_FAULT)
        panic("Fault status wrong! expected %x, got %x\n",
            TRANSLATION_SECTION_FAULT, fault_status);
    else
        trace("SUCCESS!: got fault status=%x\n", fault_status);
    domain_access_ctrl_set(DEFAULT_DOM_REG | DOM_client << (dom_heap*2));

    // done with test.
    trace("Done! Reset domain permissions\n");

    clean_reboot();
}


void notmain(void) { 
    kmalloc_init_set_start((void*)SEG_HEAP, MB(1));
    assert((uint32_t)__prog_end__ < SEG_CODE + MB(1));
    assert((uint32_t)__code_start__ >= SEG_CODE);

    // setup a data abort handler (just like last lab).
    full_except_install(0);
    full_except_set_data_abort(data_abort_handler);

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

    // Heap address where it should fault
    illegal_addr = SEG_HEAP + 0x200;

    // Should fault on something like this (e.g. will fault on 0x802c)
    // 0000802c <GET32>:
    //      802c:	e5900000 	ldr	r0, [r0]
    staff_mmu_enable();
    assert(staff_mmu_is_enabled());
    output("about to read from heap: <%x>\n", illegal_addr);
    GET32(illegal_addr);
    panic("should not reach here\n");
}
