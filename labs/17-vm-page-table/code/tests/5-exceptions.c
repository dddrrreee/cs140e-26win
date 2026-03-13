
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
#define DEFAULT_DOM_REG DOM_client << (dom_kern*2) | DOM_client << (dom_bad*2)
#define BX_LR 0xe12fff1e


// used to store the illegal address we will write.
static volatile uint32_t bad_rw_addr;
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

    domain_access_ctrl_set(DEFAULT_DOM_REG | DOM_client << (dom_bad*2));

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

    // trace("UHHH: %x\n", ifsr);
    trace("SUCCESS!: got fault status=%x\n", fault_status);
    
    domain_access_ctrl_set(DEFAULT_DOM_REG | DOM_client << (dom_bad*2));

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
    
    // ------------------------------------------------------------
    //               Make process map
    // ------------------------------------------------------------
    uint32_t dom_reg = dom_perm(1<<dom_kern, DOM_client) | dom_perm(1<<dom_bad, DOM_no_access);

    procmap_t procmap = procmap_default_mk(dom_kern);
    
    procmap_push(&procmap, pr_ent_mk(SEG_ILLEGAL, MB(1), MEM_RW, dom_bad));
    
    // Make page table kernel
    vm_pt_t* page_table = vm_map_kernel(&procmap, 1);



    // Heap address where it should fault
    bad_rw_addr = SEG_ILLEGAL;
    domain_access_ctrl_set(dom_reg);

    // --------------------------------------------------
    //              GET32 Domain Fault
    // --------------------------------------------------


    // Should fault on something like this (e.g. will fault on 0x802c)
    // 0000802c <GET32>:
    //      802c:	e5900000 	ldr	r0, [r0]
    assert(mmu_is_enabled());
    output("about to read from bad area: <%x>\n", bad_rw_addr);
    GET32(bad_rw_addr);
    trace("This is an allowed GET32: %x\n", GET32(bad_rw_addr));

    // // --------------------------------------------------
    // //              PUT32 Domain Fault
    // // --------------------------------------------------

    // Make domain no_access
    domain_access_ctrl_set(dom_reg); 

    // Should fault on something like this (e.g. will fault on 0x802c)
    // But then succeed on second
    // 0000802c <PUT32>:
    //      802c:	e5900000 	ldr	r0, [r0]
    assert(mmu_is_enabled());
    output("about to read from bad area: <%x>\n", bad_rw_addr);
    PUT32(bad_rw_addr, 0xDEADBEEF);

    // Will succeed
    PUT32(bad_rw_addr, 0xDEADBEEF);
    trace("This PUT32 succeeded: %x\n", GET32(bad_rw_addr));

    // // --------------------------------------------------
    // //              Jump
    // // --------------------------------------------------

    bad_instr_addr = SEG_ILLEGAL;
    PUT32(bad_instr_addr, BX_LR);

    // Make domain no_access
    domain_access_ctrl_set(dom_reg); 

    assert(mmu_is_enabled());
    BRANCHTO(bad_instr_addr);


}
