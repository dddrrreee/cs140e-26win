// very dumb, simple interface to wrap up watchpoints better.
// only handles a single watchpoint.
#include "rpi.h"
#include "watchpoint.h"
#include "cp-macros.h"
#include "bit-support.h"

// keep track of what we are watching.
static uint32_t watch_addr;

// was it a watchpoint fault?
//  1. use dfsr 3-64  to make sure it was a debug event.
//  2. and dscr 13-11: to make sure it was a watchpoint
int watchpt_fault_p(void) {
    uint32_t dfsr = cp15_dfsr_get();
    uint32_t dscr = cp14_dscr_get();

    int was_debug_event = (dfsr & 0b1111) == 0b0010;
    int was_watchpt_fault = (dscr >> 2 & 0b1111) == 0b0010;
    return was_debug_event && was_watchpt_fault;
}

// is it a load fault?
//  - use dfsr 3-64
int watchpt_load_fault_p(void) {
    if(!watchpt_fault_p())
        return 0;

    uint32_t dfsr = cp15_dfsr_get();
    return (dfsr >> 11 & 0b1) == 0;
}

// get the pc of the fault.
//   - p13-34: use <wfar> (see 3-12) to get the fault pc 
// important:
//   - pay attention to the comment on 13-12 to see how to adjust!
uint32_t watchpt_fault_pc(void) {
    uint32_t wfr = cp14_wfar_get();
    return wfr - 8;
}

// get the data address that caused the fault.
// use <far> 3-68 to get the fault addr.
uint32_t watchpt_fault_addr(void) {
    return cp15_far_get();
}

// set a watch-point on <addr>: 
//  1. enable cp14 if not enabled.  
//     - MAKE SURE TO DO THIS FIRST.
//  2. set wcr0 (13-21), wvr0 (13-20)
//     - don't rmw -- just set it directly.
// Important: 
//  - make sure you handle subword accesses! 
int watchpt_on(uint32_t addr) {

    watch_addr = addr;
    
    // 0. Enable CP14
    // cp14_dscr_set(1 << 15);
    uint32_t dscr = cp14_dscr_get();
    dscr |= (1 << 15); // The Monitor debug-mode enable bit
    dscr &= ~(1 << 14); // Mode select bit (selects monitor-debug mode)
    cp14_dscr_set(dscr);
    
    // 1. Read the WCR.
    uint32_t wcr = cp14_wcr_get();

    // 2. Clear the WCR[0] enable watchpoint bit in the read word 
    //    and write it back to the WCR. Now the watchpoint is disabled.
    wcr &= ~1;
    cp14_wcr_set(wcr);
    
    // 3. Write the DMVA to the WVR register.
    cp14_wvr_set(watch_addr);
    
    
    
    // 4. Write to the WCR with its fields set as follows:
    wcr = cp14_wcr_get();

    wcr &= ~(1 << 20);          // WCR[20] enable linking bit cleared
    wcr &= ~(0b11 << 14);       // WCR[15:14] Matches in secure and non-secure
    wcr |= 0b1111 << 5;         // WCR[8:5] Byte address select (+1-3 offsets also trigger)
    wcr |= 0b11 << 3;           // WCR[4:3] Load/store access (Load or store)
    wcr |= 0b11 << 1;           // WCR[2:1] Supervisor access (either privileged or user)
    wcr |= 0b1;                 // WCR[0] enable watchpoint bit set

    // Need 0 and 4:3 set

    cp14_wcr_set(wcr);

    prefetch_flush();

    return 1;
}

// turn off watchpoint:
//   - check that <addr> is what we were watching.
//   - clear wcr0
int watchpt_off(uint32_t addr) {
    if(addr != watch_addr)
        panic("disabling invalid watchpoint %x, tracking %x\n", 
            addr, watch_addr);

    cp14_wcr_set(0); // THIS MAY BE WRONG

    prefetch_flush();
    
    return 1;
}
