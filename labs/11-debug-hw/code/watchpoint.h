#ifndef __WATCH_PT_H__
#define __WATCH_PT_H__

// set watchpt on <addr>
int watchpt_on(uint32_t addr);
int staff_watchpt_on(uint32_t addr);

// disable watchpt on <addr>
int watchpt_off(uint32_t addr);
int staff_watchpt_off(uint32_t addr);

// called from fault handler --- really should
// merge together for speed.

// is it a watchpt fault?
//   1. was a debug instruction fault
//   2. watchpt occured
int watchpt_fault_p(void);
int staff_watchpt_fault_p(void);

// get the watchpt fault addr
uint32_t watchpt_fault_addr(void);
uint32_t staff_watchpt_fault_addr(void);

// get the watchpt pc [won't be the exception pc]
uint32_t watchpt_fault_pc(void);
uint32_t staff_watchpt_fault_pc(void);

// was the fault from a load?
int watchpt_load_fault_p(void);
int staff_watchpt_load_fault_p(void);

uint32_t watchpt_fault_addr(void);
uint32_t staff_watchpt_fault_addr(void);

#endif
