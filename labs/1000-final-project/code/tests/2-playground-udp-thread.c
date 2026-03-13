#include "rpi.h"
#include "spi.h"
#include "../net-stack/netif/w5500.h"
#include "../net-stack/inet.h"

#include "test-setup.h"
#include "rpi-thread.h"
#include "redzone.h"
#include "gpio.h"

static int should_exit = 0;

void inet_thread(void* arg) {
    while(1) {
        inet_poll_frame(1);
        
        if (should_exit)
            rpi_exit(0);

        rpi_yield();
    }
}

void led_thread(void* arg) {
    gpio_set_output(GPIO_HAT_LED);

    unsigned i = 0;
    while (1) {
        i++;
        
        gpio_set_on(GPIO_HAT_LED);

        uint32_t start = timer_get_usec();
        while (timer_get_usec() - start < 1000000) {
            rpi_yield();
        }

        gpio_set_off(GPIO_HAT_LED);

        start = timer_get_usec();
        while (timer_get_usec() - start < 1000000) {
            rpi_yield();
        }

        

        if (i == 10){
            should_exit = 1;
            trace("TIME IS UP. NO MORE INET FOR YOU\n");
            rpi_exit(0);
        }
    }
}

void notmain(void) { 
    unsigned oneMB = 1024*1024;
    kmalloc_init_set_start((void*)oneMB, oneMB);

    redzone_init();
    // this first one should pass.
    redzone_check("initialized redzone");
    printk("heap_start=%p\n", kmalloc_heap_start());
    printk("heap_end=%p\n", kmalloc_heap_end());

    w5500_conf_t config = get_test_w5500_config();
    w5500_t nic;
    w5500_init(&nic, &config);

    verbose_t v = inet_verbosity_init();
    v.arp = 1;
    // v.icmp = 1;
    v.udp = 1;
    // v.data_link_send = 1;
    v.all = 1;
    inet_init(&nic, &v); // 0 IS THE ONE CLOSEST TO IMU


    // inet_udp_add_port_handler(src_port, port_handler);

    trace("Polling on ipv4={%d.%d.%d.%d} with mac={%X:%X:%X:%X:%X:%X}\n\n",
        inet_get_ipv4_addr()[0],
        inet_get_ipv4_addr()[1],
        inet_get_ipv4_addr()[2],
        inet_get_ipv4_addr()[3],
        inet_get_hw_addr()[0],
        inet_get_hw_addr()[1],
        inet_get_hw_addr()[2],
        inet_get_hw_addr()[3],
        inet_get_hw_addr()[4],
        inet_get_hw_addr()[5]
    );

    gpio_set_output(GPIO_HAT_LED);
    gpio_set_on(GPIO_HAT_LED);

    
    rpi_fork(inet_thread, NULL);
    rpi_fork(led_thread, NULL);

    rpi_thread_start();
}