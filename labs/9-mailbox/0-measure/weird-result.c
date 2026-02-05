// measure a weird thing about instruction alignment.
//
// if you care about speed and measuring actual cycle counts:
//   1. ignoring seemingly small detail can make all low-level 
//      experiments completely broken (llms always mess this up),
//   2. lead to hours of "what is happening" debugging.
//
#include "rpi.h"
#include "cycle-count.h" // libpi/include/cycle-count.h

void measure_weird(const char *msg);

void notmain(void) {
    printk("\nmeasuring cost of different operations (pi A+ = 700 cyc / usec)\n");
    cycle_cnt_init();

    measure_weird("cache off");
    caches_enable();
    measure_weird("cache on (first run)");
    measure_weird("cache on (second run)");

    // note: if you look at our cache disable it's sketch: 
    //  - doesn't invalidate icache or btb
    //  - this is a sin.  don't do that.  we get lucky here.
    caches_disable();
    measure_weird("cache back off");
}
    
// measure a weirdness with alignment.
void measure_weird(const char *msg) {
    printk("\n\nmeasuring a weird feature: <%s>\n", msg);

    asm volatile(".align 5");
    let s = cycle_cnt_read();
    // uncached = 8 cycles
    let e = cycle_cnt_read();
    printk("\tempty cycle count = measurement: %d cycles\n", e-s);

    asm volatile(".align 5");
    asm volatile("nop");    // 1
    asm volatile("nop");    // 2
    asm volatile("nop");    // 3
    asm volatile("nop");    // 4
    asm volatile("nop");    // 5
    asm volatile("nop");    // 6
    asm volatile("nop");    // 7 (7*4=28 bytes since .align)

    // uncached = about 36: "why??"
    s = cycle_cnt_read();
    e = cycle_cnt_read();
    printk("\tempty cycle count = measurement: %d cycles\n", e-s);
}
