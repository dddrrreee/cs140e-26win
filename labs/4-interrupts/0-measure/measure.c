// measure the cost (in cycles) of different operations.
// good way to get intuition, and to see how it breaks.
//
// NOTE: we mostly only measure one op at a time without 
// handling alignment or cache effects --- worth seeing
// how clean you can get the readings.
#include "rpi.h"
#include "cycle-count.h" // libpi/include/cycle-count.h

void measure(const char *msg);

void notmain(void) {
    printk("\nmeasuring cost of different operations (pi A+ = 700 cyc / usec)\n");
    cycle_cnt_init();

    measure("cache off");

    caches_enable();
    measure("with icache on first run");
    // Q: if you run this?
    // measure("with icache on second run");
}

uint32_t measure_put32(volatile uint32_t *ptr) ;
uint32_t measure_get32(volatile uint32_t *ptr) ;
uint32_t measure_ptr_write(volatile uint32_t *ptr);
uint32_t measure_ptr_read(volatile uint32_t *ptr);
    
void measure(const char *msg) {

    printk("------------------------------------------------------       \n");
    printk("measuring: <%s>\n", msg);
    
    uint32_t x;

    // Q: try switching --- does this pattern make a difference for
    //    any measurement?
#if 0
    let t0 = measure_put32(&x);
    let t1 = measure_put32(&x);
    printk("\tcall to PUT32  =\t%d cycles\n", t0);
    printk("\tcall to PUT32  =\t%d cycles\n", t1);
#else

    printk("\tcall to PUT32  =\t%d cycles\n", measure_put32(&x));
    printk("\tcall to PUT32  =\t%d cycles\n", measure_put32(&x));
#endif

    enum { BASE = 0x20200000 };
    // periph write to try with a set0 (safe)
    let set0 = (void*)(BASE + 0x1C);
    // periph read from level (safe)
    let level0 = (void*)(BASE + 0x34);

    // Q: why not much difference if you double?   what if you
    // defer the printk?
    printk("\tcall to GET32  =\t%d cycles\n", measure_get32(&x));
    printk("\tptr write      =\t%d cycles\n", measure_ptr_write(&x));
    printk("\tptr read       =\t%d cycles\n", measure_ptr_read(&x));
    printk("\tperiph write   =\t%d cycles\n", measure_ptr_write(set0));
    printk("\tperiph read    =\t%d cycles\n", measure_ptr_read(level0));
    printk("\tnull ptr write =\t%d cycles\n", measure_ptr_write(0));
    printk("\tnull ptr read  =\t%d cycles\n", measure_ptr_read(0));

    printk("                ----------------------------       \n");
    asm volatile(".align 5");
    let s = cycle_cnt_read();
    *(volatile uint32_t *)0 = 4;
    let e = cycle_cnt_read();
    printk("\taligned null pointer write =%d cycles\n", e-s);

    // use macro <TIME_CYC_PRINT> to clean up a bit.
    //  see: libpi/include/cycle-count.h
    TIME_CYC_PRINT("\tread/write barrier", dev_barrier());
    TIME_CYC_PRINT("\tread barrier      ", dmb());
    TIME_CYC_PRINT("\tsafe timer        ", timer_get_usec());
    TIME_CYC_PRINT("\tunsafe timer      ", timer_get_usec_raw());
    // macro expansion messes up the printk
    printk("\t<cycle_cnt_read()>: %d\n", TIME_CYC(cycle_cnt_read()));
    printk("------------------------------------------------------       \n");
}

// pull these out so we can see the machine code
uint32_t measure_put32(volatile uint32_t *ptr) {
    asm volatile(".align 5");
    let s = cycle_cnt_read();
        put32(ptr,0);
    let e = cycle_cnt_read();
    return e-s;
}
uint32_t measure_get32(volatile uint32_t *ptr) {
    asm volatile(".align 5");
    // simple example to measure GET32
    let s = cycle_cnt_read();
        get32(ptr);
    let e = cycle_cnt_read();
    return e-s;
}

uint32_t measure_ptr_write(volatile uint32_t *ptr) {
    asm volatile(".align 5");
    // a bit weird :)
    let s = cycle_cnt_read();
        *ptr = 4;
    let e = cycle_cnt_read();
    return e-s;
}

uint32_t measure_ptr_read(volatile uint32_t *ptr) {
    asm volatile(".align 5");
    let s = cycle_cnt_read();
        uint32_t x = *ptr;
    let e = cycle_cnt_read();
    return e-s;
}
