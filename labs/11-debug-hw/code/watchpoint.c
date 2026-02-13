// very dumb, simple interface to wrap up watchpoints better.
// only handles a single watchpoint.
#include "rpi.h"
#include "watchpoint.h"

// keep track of what we are watching.
static uint32_t watch_addr;

// was it a watchpoint fault?
//  1. use dfsr 3-64  to make sure it was a debug event.
//  2. and dscr 13-11: to make sure it was a watchpoint
int watchpt_fault_p(void) {
    return staff_watchpt_fault_p();
}

// is it a load fault?
//  - use dfsr 3-64
int watchpt_load_fault_p(void) {
    if(!watchpt_fault_p())
        return 0;
    return staff_watchpt_load_fault_p();
}

// get the pc of the fault.
//   - p13-34: use <wfar> (see 3-12) to get the fault pc 
// important:
//   - pay attention to the comment on 13-12 to see how to adjust!
uint32_t watchpt_fault_pc(void) {
    return staff_watchpt_fault_pc();
}

// get the data address that caused the fault.
// use <far> 3-68 to get the fault addr.
uint32_t watchpt_fault_addr(void) {
    return staff_watchpt_fault_addr();
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
    return staff_watchpt_on(addr);
}

// turn off watchpoint:
//   - check that <addr> is what we were watching.
//   - clear wcr0
int watchpt_off(uint32_t addr) {
    if(addr != watch_addr)
        panic("disabling invalid watchpoint %x, tracking %x\n", 
            addr, watch_addr);
    return staff_watchpt_off(addr);
}
