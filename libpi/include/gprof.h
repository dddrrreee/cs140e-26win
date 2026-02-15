#ifndef __GPROF_H__
#define __GPROF_H__

// ** MADE BY COLIN for declaring GPROF


/*************************************************************
 * gprof implementation:
 *	- allocate a table with one entry for each instruction.
 *	- gprof_init(void) - call before starting.
 *	- gprof_inc(pc) will increment pc's associated entry.
 *	- gprof_dump will print out all samples.
 */

// - compute <pc_min>, <pc_max> using the 
//   <libpi/memmap> symbols: 
//   - use for bounds checking.
// - allocate <hist> with <kmalloc> using <pc_min> and
//   <pc_max> to compute code size.
unsigned gprof_init(void);

// increment histogram associated w/ pc.
//    few lines of code
void gprof_inc(unsigned pc);

// print out all samples whose count > min_val
//
// make sure gprof does not sample this code!
// we don't care where it spends time.
//
// how to validate:
//  - take the addresses and look in <gprof.list>
//  - we expect pc's to be in GET32, PUT32, different
//    uart routines, or rpi_wait.  (why?)
void gprof_dump(unsigned min_val);



#endif