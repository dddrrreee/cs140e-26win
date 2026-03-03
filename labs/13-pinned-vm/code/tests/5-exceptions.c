
#include "rpi.h"
#include "../pinned-vm.h"
#include "../memmap-default.h"
#include "full-except.h"
#include "memmap.h"

#include "cp-macros.h"

#define MB(x) ((x)*1024*1024)
#define dom_heap 3
#define asid_heap 13
#define DOMAIN_SECTION_FAULT 0b01001
#define DEFAULT_DOM_REG DOM_client << (dom_kern*2) | DOM_client << (dom_heap*2)
#define BX_LR 0xe12fff1e

// used to store the illegal address we will write.
static volatile uint32_t heap_rw_addr;
static volatile uint32_t bad_instr_addr;
static volatile uint32_t GET32_ADDR = (uint32_t)&GET32;
static volatile uint32_t PUT32_ADDR = (uint32_t)&PUT32;

static void get_data_abort_handler(regs_t *r) {
    uint32_t pc = r->regs[15];

    // make sure we faulted on the GET32 address OR PUT32 address.
    if(pc == GET32_ADDR)
        trace("SUCCESS! Got GET32 fault on %x\n",
            GET32_ADDR, pc);
    else if(pc == PUT32_ADDR)
        trace("SUCCESS! Got PUT32 fault on %x\n",
            GET32_ADDR, pc);
    else
        panic("illegal fault! expected %x (GET32) or %x (PUT32), got %x\n",
            GET32_ADDR, PUT32_ADDR, pc);

    // trace("dfsr: %x\n", cp15_dfsr_get());
    // trace("ifsr: %x\n", cp15_ifsr_get());
    uint32_t dfsr = cp15_dfsr_get();
    uint32_t fault_status = 
        ( (dfsr >> 6) & 0b1 )
        | (dfsr & 0b1111);
    
    if(fault_status == DOMAIN_SECTION_FAULT)
        trace("SUCCESS!: got fault status=%x\n", fault_status);
    else
        panic("Fault status wrong! expected %x, got %x\n",
            DOMAIN_SECTION_FAULT, fault_status);

    domain_access_ctrl_set(DEFAULT_DOM_REG | DOM_client << (dom_heap*2));

    // done with test.
    trace("Done with data_abort_handler() !  Reset domain permissions\n");

    switchto(r);
}

static void prefetch_handler(regs_t *r) {
    uint32_t pc = r->regs[15];

    // make sure we faulted on the GET32 address.
    if(pc != bad_instr_addr)
        panic("illegal fault!  expected %x, got %x\n",
            bad_instr_addr, pc);
    else
        trace("SUCCESS!: got a fault on pc=%x\n", pc);

    uint32_t ifsr = cp15_ifsr_get();
    uint32_t fault_status = 
        ( (ifsr >> 7) & 0b1000 )
        | (ifsr & 0b1111);

    trace("UHHH: %x\n", ifsr);
    trace("SUCCESS!: got fault status=%x\n", fault_status);
    
    domain_access_ctrl_set(DEFAULT_DOM_REG | DOM_client << (dom_heap*2));

    // done with test.
    trace("Done with prefetch_handler() !  Reset domain permissions\n");
    
    switchto(r);
}


void notmain(void) { 
    kmalloc_init_set_start((void*)SEG_HEAP, MB(1));
    assert((uint32_t)__prog_end__ < SEG_CODE + MB(1));
    assert((uint32_t)__code_start__ >= SEG_CODE);

    // setup a data abort handler (just like last lab).
    full_except_install(0);
    full_except_set_data_abort(get_data_abort_handler);
    full_except_set_prefetch(prefetch_handler);

    assert(!mmu_is_enabled());
    pin_t dev  = pin_mk_global(dom_kern, no_user, MEM_device);
    pin_t kern = pin_mk_global(dom_kern, no_user, MEM_uncached);
    pin_t heap = pin_mk_global(dom_heap, no_user, MEM_uncached); // New domain ID

    staff_mmu_init();
    pin_mmu_sec(0, SEG_CODE, SEG_CODE, kern);
    pin_mmu_sec(1, SEG_HEAP, SEG_HEAP, heap); 
    pin_mmu_sec(2, SEG_STACK, SEG_STACK, kern); 
    pin_mmu_sec(3, SEG_INT_STACK, SEG_INT_STACK, kern); 
    pin_mmu_sec(4, SEG_BCM_0, SEG_BCM_0, dev); 
    pin_mmu_sec(5, SEG_BCM_1, SEG_BCM_1, dev); 
    pin_mmu_sec(6, SEG_BCM_2, SEG_BCM_2, dev); 

    // Heap address where it should fault
    heap_rw_addr = SEG_HEAP;

    // --------------------------------------------------
    //              GET32 Domain Fault
    // --------------------------------------------------

    // Make domain no_access
    unsigned dom_reg = DOM_manager << (dom_kern*2) | DOM_no_access << (dom_heap*2);
    pin_mmu_init(dom_reg);

    // Should fault on something like this (e.g. will fault on 0x802c)
    // 0000802c <GET32>:
    //      802c:	e5900000 	ldr	r0, [r0]
    staff_mmu_enable();
    assert(staff_mmu_is_enabled());
    output("about to read from heap: <%x>\n", heap_rw_addr);
    GET32(heap_rw_addr);
    trace("This is an allowed GET32: %x\n", GET32(heap_rw_addr));

    // --------------------------------------------------
    //              PUT32 Domain Fault
    // --------------------------------------------------

    // Make domain no_access
    dom_reg = DOM_manager << (dom_kern*2) | DOM_no_access << (dom_heap*2);
    pin_mmu_init(dom_reg);

    // Should fault on something like this (e.g. will fault on 0x802c)
    // But then succeed on second
    // 0000802c <PUT32>:
    //      802c:	e5900000 	ldr	r0, [r0]
    assert(staff_mmu_is_enabled());
    output("about to read from heap: <%x>\n", heap_rw_addr);
    PUT32(heap_rw_addr, 0xDEADBEEF);

    // Will succeed
    PUT32(heap_rw_addr, 0xDEADBEEF);
    trace("This PUT32 succeeded: %x\n", GET32(heap_rw_addr));

    // --------------------------------------------------
    //              Jump
    // --------------------------------------------------

    bad_instr_addr = SEG_HEAP;
    PUT32(bad_instr_addr, BX_LR);

    // Make domain no_access
    dom_reg = DOM_manager << (dom_kern*2) | DOM_no_access << (dom_heap*2);
    pin_mmu_init(dom_reg);
    

    assert(staff_mmu_is_enabled());
    BRANCHTO(bad_instr_addr);


}
