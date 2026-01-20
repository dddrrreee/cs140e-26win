/*
 * use interrupts to implement a simple statistical profiler.
 *	- interrupt code is a replication of ../timer-int/timer.c
 *	- you'll need to implement kmalloc so you can allocate 
 *	  a histogram table from the heap.
 *	- implement functions so that given a pc value, you can increment
 *	  its associated count
 */
#include "rpi.h"

// pulled the interrupt code into these header files.
#include "rpi-interrupts.h"
#include "timer-int.h"

// defines C externs for the labels defined by the linker script
// libpi/memmap.
// 
// you can use these to get the size of the code segment, 
// data segment, etc.
#include "memmap.h"

/*************************************************************
 * gprof implementation:
 *	- allocate a table with one entry for each instruction.
 *	- gprof_init(void) - call before starting.
 *	- gprof_inc(pc) will increment pc's associated entry.
 *	- gprof_dump will print out all samples.
 */

static unsigned hist_n, pc_min, pc_max;
static volatile unsigned *hist = 0;

// - compute <pc_min>, <pc_max> using the 
//   <libpi/memmap> symbols: 
//   - use for bounds checking.
// - allocate <hist> with <kmalloc> using <pc_min> and
//   <pc_max> to compute code size.
static unsigned gprof_init(void) {
    todo("allocate <hist> using <kmalloc>.  initialize etc\n");

    // TODO: not sure which one is at the end
    pc_min = (unsigned)__code_start__;
    pc_max = (unsigned)__code_end__;
    hist_n = pc_max - pc_min;
    hist = kmalloc(hist_n);

    return hist_n;
}

// increment histogram associated w/ pc.
//    few lines of code
static void gprof_inc(unsigned pc) {
    assert(pc >= pc_min && pc <= pc_max);
    todo("make sure you bounds check\n");

    // TODO: check
    unsigned index = (pc - pc_min) / 4; // don't have to make this volatile because it can be optimized I think
    hist[index]++;
}

// print out all samples whose count > min_val
//
// make sure gprof does not sample this code!
// we don't care where it spends time.
//
// how to validate:
//  - take the addresses and look in <gprof.list>
//  - we expect pc's to be in GET32, PUT32, different
//    uart routines, or rpi_wait.  (why?)
static void gprof_dump(unsigned min_val) {
    todo("make sure you don't trace this routine!\n");
    
    // TODO: verify that this works with incrementing
    for (volatile unsigned* ptr = hist; ptr < hist + hist_n; ptr++)
        if (*ptr > min_val)
            printk("%d\n", ptr);
    
}

/**************************************************************
 * timer interrupt code from before, now calls gprof update.
 */
// Q: if you make not volatile?
static volatile unsigned cnt;
static volatile unsigned period;

// client has to define this.
void interrupt_vector(unsigned pc) {
    dev_barrier();
    unsigned pending = GET32(IRQ_basic_pending);
    if((pending & ARM_Timer_IRQ) == 0)
        return;

    PUT32(ARM_Timer_IRQ_Clear, 1);
    cnt++;

    // increment the counter for <pc>.
    gprof_inc(pc);

    // this doesn't need to stay here.
    static unsigned last_clk = 0;
    unsigned clk = timer_get_usec();
    period = last_clk ? clk - last_clk : 0;
    last_clk = clk;

    dev_barrier();
}

// trivial program to test gprof implementation.
// 	- look at output: do you see weird patterns?
void notmain() {
    interrupt_init();
    timer_init(16, 0x100);

    // Q: if you move these below interrupt enable?
    uint32_t oneMB = 1024*1024;
    kmalloc_init_set_start((void*)oneMB, oneMB);
    gprof_init();

    printk("gonna enable ints globally!\n");
    enable_interrupts();

    // caches_enable(); 	// Q: what happens if you enable cache?
    unsigned iter = 0;
    while(cnt<200) {
        printk("iter=%d: cnt = %d, period = %dusec, %x\n",
                iter,cnt, period,period);
        iter++;
        if(iter % 10 == 0)
            gprof_dump(2);
    }
}
