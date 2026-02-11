#include "rpi.h"
#include "mbox.h"

uint32_t rpi_temp_get(void) ;

#include "cycle-count.h"

// compute cycles per second using
//  - cycle_cnt_read();
//  - timer_get_usec();
unsigned cyc_per_sec(void) {
    todo("implement this!\n");
}

void check_clocks() {
    // UART Clock
    uint32_t clock_id = 0x2; 
    uint32_t value;

    // Getting current clock speed
    value = rpi_clock_curhz_get(clock_id);
    output("Clock %d rate = %d Hz, %d MHz\n", clock_id, value, value/1000000);

    printk("\nBefore setting clock:\n");
    // Before setting
    output("Max clock %d rate = %d Hz\n", clock_id, rpi_clock_maxhz_get(clock_id));
    output("Min clock %d rate = %d Hz\n", clock_id, rpi_clock_minhz_get(clock_id));
    output("Measured clock %d rate = %d Hz\n", clock_id, rpi_clock_realhz_get(clock_id));

    // Setting

    rpi_clock_hz_set(clock_id, rpi_clock_maxhz_get(clock_id) / 10); // WHY THIS?
    
    // After setting
    printk("\nAfter setting clock to max:\n");
    output("Measured clock %d rate = %d Hz\n", clock_id, rpi_clock_realhz_get(clock_id));

    rpi_clock_hz_set(clock_id, 50000);

    printk("\nAfter setting clock to small:\n");
    output("Measured clock %d rate = %d Hz\n", clock_id, rpi_clock_realhz_get(clock_id));
}


void notmain(void) { 
    output("mailbox serial number = %llx\n", rpi_get_serialnum()); //  0xa8249570
    
    output("mailbox revision number = %x\n", rpi_get_revision()); //  0x9000c1
    output("mailbox model number = %x\n", rpi_get_model());
    
    uint32_t size = rpi_get_memsize();
    output("mailbox physical mem: size=%d (%dMB)\n", 
        size, 
        size/(1024*1024));
    
    // print as fahrenheit
    unsigned x = rpi_temp_get();
    
    // convert <x> to C and F
    unsigned C = 0, F = 0;
    C = x/1000;
    F = (C * 9 / 5) + 32;
    output("mailbox temp = %x, C=%d F=%d\n", x, C, F); 

    output("RPi core voltage = %d mV\n", rpi_get_voltage(1)); // core

        
    
    
    // // Increasing memory size
    uint32_t mem_handle = rpi_allocate_memory(0x8000000);
    output("memory handle = %x ?\n", mem_handle);
    
    size = rpi_get_memsize();
    output("mailbox physical mem: size=%d (%dMB)\n", 
        size, 
        size/(1024*1024));
    // todo("implement the rest");
    
    check_clocks();
    // value = rpi_clock_curhz_get(clock_id);
    // output("Clock %d rate = %d Hz, %d MHz\n", clock_id, value, value/1000000);

    // rpi_clock_hz_set(3, 700000000);

    // value = rpi_clock_curhz_get(clock_id);
    // output("Clock %d rate = %d Hz, %d MHz\n", clock_id, value, value/1000000);

    // for (int i = 0; i < 0xf; i++) {
    //     uint32_t value = rpi_clock_curhz_get(i);
    //     output("Clock %d rate = %d Hz, %d MHz\n", i, value, value/1000000);
    // }
    
    // todo("do overclocking!\n");


}
