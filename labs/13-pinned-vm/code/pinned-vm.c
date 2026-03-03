// put your code here.
//
#include "rpi.h"
#include "libc/bit-support.h"

// has useful enums and helpers.
#include "vector-base.h"
#include "pinned-vm.h"
#include "mmu.h"
#include "procmap.h"


// ** DO WE NEED THIS?
#include "rpi-interrupts.h"

// generate the _get and _set methods.
// (see asm-helpers.h for the cp_asm macro 
// definition)
// arm1176.pdf: 3-149

#include "asm-helpers.h"

// Page 3-153
cp_asm_get(lockdown_index, p15, 5, c15, c4, 2); // Read TLB Lockdown Index Register
cp_asm_set(lockdown_index, p15, 5, c15, c4, 2); // Write TLB Lockdown Index Register
cp_asm_get(lockdown_va, p15, 5, c15, c5, 2); // Read TLB Lockdown VA Register
cp_asm_set(lockdown_va, p15, 5, c15, c5, 2); // Write TLB Lockdown VA Register
cp_asm_get(lockdown_pa, p15, 5, c15, c6, 2); // Read TLB Lockdown PA Register
cp_asm_set(lockdown_pa, p15, 5, c15, c6, 2); // Write TLB Lockdown PA Register
cp_asm_get(lockdown_attr, p15, 5, c15, c7, 2); // Read TLB Lockdown Attributes Register
cp_asm_set(lockdown_attr, p15, 5, c15, c7, 2); // Write TLB Lockdown Attributes Register

cp_asm_set(va_to_pa_priv, p15, 0, c7, c8, 1); // Write VA-to-PA translation
// cp_asm_set(va_to_pa_user, p15, 0, c7, c8, 3); // Write VA-to-PA translation

cp_asm_get(pa_priv, p15, 0, c7, c4, 0); // Read PA translation
// cp_asm_get(pa_user, p15, 0, c7, c4, 2); // Read PA translation


static void *null_pt = 0;

// should we have a pinned version?
void domain_access_ctrl_set(uint32_t d) {
    staff_domain_access_ctrl_set(d);
}

// ****** GLOBAL ENUMS ******

// fill this in based on the <1-test-basic-tutorial.c>
// NOTE: 
//    you'll need to allocate an invalid page table
//    CPU will check TLB for VA, if not found, go to page table, but it needs to fault if it goes there
void pin_mmu_init(uint32_t domain_reg) {
    // staff_pin_mmu_init(domain_reg); return;

    // Allocate an invalid 16 KB page table
    null_pt = kmalloc_aligned(4096*4, 1<<14);
    assert((uint32_t)null_pt % (1<<14) == 0);

    domain_access_ctrl_set(domain_reg); 
}

// do a manual translation in tlb:
//   1. store result in <result>
//   2. return 1 if entry exists, 0 otherwise.
//
// NOTE: mmu must be on (confusing).
int tlb_contains_va(uint32_t *result, uint32_t va) {
    // return staff_tlb_contains_va(result, va); 
    assert(mmu_is_enabled());

    // 3-79
    assert(bits_get(va, 0,2) == 0);

    va_to_pa_priv_set(va);
    *result = pa_priv_get();

    uint32_t zero_bit_set = *result & 1;

    // Mask out non-addr bits
    *result &= ~0xFFFFF;
    // Do we just or these?
    *result |= (va & 0xFFFFF);
    
    return !zero_bit_set;
}

// map <va>-><pa> at TLB index <idx> with attributes <e>
void pin_mmu_sec(unsigned idx,  
                uint32_t va, 
                uint32_t pa,
                pin_t e) {
    // staff_pin_mmu_sec(idx, va, pa, e); return;


    demand(idx < 8, lockdown index too large);
    // lower 20 bits should be 0.
    demand(bits_get(va, 0, 19) == 0, only handling 1MB sections);
    demand(bits_get(pa, 0, 19) == 0, only handling 1MB sections);

    // debug("about to map %x->%x\n", va,pa);

    // these will hold the values you assign for the tlb entries.
    volatile uint32_t x, va_ent, pa_ent, attr = 0;
    // todo("assign these variables!\n");

    disable_interrupts(); // ? Should we do it? Written on last page
    // ----- Select required entry ----- 
    lockdown_index_set(idx & 0xFF);
    // ----- VA Register ----- 
    va_ent = va & 0xFFFFF000;                    // Start with VA
    va_ent |= (e.G & 1) << 9;       // Is global

    if (!e.G)
        va_ent |= e.asid & 0xFF;    // ASID only set for non-global entries

    lockdown_va_set(va_ent);

    // ----- Attributes Register ----- 
    attr = e.mem_attr << 1;
    attr |= (e.dom & 0b1111) << 7;   // Took the longest to do
    lockdown_attr_set(attr);
    
    // ----- PA Register ----- 
    pa_ent = pa & 0xFFFFF000;                // Start with PA for [12:31]
    pa_ent |= e.pagesize << 6;
    pa_ent |= e.AP_perm << 1;
    pa_ent |= 1;                // Valid bit set automatically though

    lockdown_pa_set(pa_ent);

    enable_interrupts(); // ? Should we do it? Written on last page

#if 1
    if((x = lockdown_va_get()) != va_ent)
        panic("lockdown va: expected %x, have %x\n", va_ent,x);
    if((x = lockdown_pa_get()) != pa_ent)
        panic("lockdown pa: expected %x, have %x\n", pa_ent,x);
    if((x = lockdown_attr_get()) != attr)
        panic("lockdown attr: expected %x, have %x\n", attr,x);
#endif
}

// check that <va> is pinned.  
int pin_exists(uint32_t va, int verbose_p) {
    // return staff_pin_exists(va, verbose_p); 
    if(!mmu_is_enabled())
        panic("XXX: i think we can only check existence w/ mmu enabled\n");

    uint32_t r;
    if(tlb_contains_va(&r, va)) {
        assert(va == r);
        return 1;
    } else {
        if(verbose_p) 
            output("TLB should have %x: returned %x [reason=%b]\n", 
                va, r, bits_get(r,1,6));
        return 0;
    }
}

// look in test <1-test-basic.c> to see what to do.
// need to set the <asid> before turning VM on and 
// to switch processes.
void pin_set_context(uint32_t asid) {
    // staff_pin_set_context(asid); return;
    // put these back
    demand(asid > 0 && asid < 64, invalid asid);
    demand(null_pt, must setup null_pt --- look at tests);

    enum { PID = 128 };
    staff_mmu_set_ctx(PID, asid, null_pt);

}

void pin_clear(unsigned idx)  {
    // staff_pin_clear(idx); return;
    lockdown_index_set(idx & 0xFF);
    lockdown_va_set(0);
    lockdown_attr_set(0);
    lockdown_pa_set(0);
}

void lockdown_print_entry(unsigned idx) {
    trace_nofn("   idx=%d\n", idx);
    lockdown_index_set(idx);
    uint32_t va_ent = lockdown_va_get();
    uint32_t pa_ent = lockdown_pa_get();
    uint32_t attr = lockdown_attr_get();
    uint32_t v = bit_get(pa_ent, 0);

    if(!v) {
        trace_nofn("     [invalid entry %d]\n", idx);
        return;
    }

    // 3-149
    uint32_t va = (va_ent >> 12) & 0xFFFFF; 
    uint32_t G = (va_ent >> 9) & 1;
    uint32_t asid = va_ent & 0xFF;
    trace_nofn("     va_ent=%x: va=%x|G=%d|ASID=%d\n",
        va_ent, va, G, asid);

    // 3-150
    uint32_t pa = (pa_ent >> 12) & 0xFFFFF;
    uint32_t nsa = (pa_ent >> 9) & 1;
    uint32_t nstid = (pa_ent >> 8) & 1;
    uint32_t size = (pa_ent >> 6) & 0b11;
    uint32_t apx = (pa_ent >> 1) & 0b111;
    trace_nofn("     pa_ent=%x: pa=%x|nsa=%d|nstid=%d|size=%b|apx=%b|v=%d\n",
                pa_ent, pa, nsa,nstid,size, apx,v);

    // 3-151
    uint32_t dom = (attr >> 7) & 0b1111;
    uint32_t xn = (attr >> 6) & 1;
    uint32_t tex = (attr >> 3) & 0b111;
    uint32_t C = (attr >> 2) & 1;
    uint32_t B = (attr >> 1) & 1;
    trace_nofn("     attr=%x: dom=%d|xn=%d|tex=%b|C=%d|B=%d\n",
            attr, dom,xn,tex,C,B);
}

void lockdown_print_entries(const char *msg) {
    trace_nofn("-----  <%s> ----- \n", msg);
    trace_nofn("  pinned TLB lockdown entries:\n");
    for(int i = 0; i < 8; i++)
        lockdown_print_entry(i);
    trace_nofn("----- ---------------------------------- \n");
}
