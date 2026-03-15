#include "rpi.h"
#include "../net-stack/netif/w5500.h"
#include "../net-stack/inet.h"

#include "test-setup.h"
#include "project.h"
#include "rpi-thread.h"
#include "redzone.h"

void notmain(void) { 
    unsigned oneMB = 1024*1024;
    kmalloc_init_set_start((void*)oneMB, oneMB);

    redzone_init();
    // this first one should pass.
    redzone_check("initialized redzone");

    w5500_conf_t config = get_test_w5500_config();
    w5500_t nic;
    w5500_init(&nic, &config);

    verbose_t v = inet_verbosity_init();
    v.arp = 1;
    // v.icmp = 1;
    v.udp = 1;
    // v.data_link_send = 1;
    v.all = 1;
    inet_init(&nic, &v);

    uart_init();


    // Adding Handlers
    inet_udp_add_port_handler(PARTHIV_LED_PORT, parthiv_led_handler);
    inet_udp_add_port_handler(ONBOARD_LED_PORT, onboard_led_handler);


    // Showing status
    printk("Polling on ipv4={%d.%d.%d.%d} with mac={%X:%X:%X:%X:%X:%X}\n",
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
    printk("Opening port %d to Parthiv LED thread and port %d to onboard LED thread...\n\n",
        PARTHIV_LED_PORT, ONBOARD_LED_PORT);

    // HAT starts on to show the network is up
    gpio_set_output(GPIO_HAT_LED);
    gpio_set_on(GPIO_HAT_LED);

    
    rpi_fork(network_thread, NULL);
    rpi_fork(onboard_led_thread, NULL);
    rpi_fork(parthiv_led_thread, NULL);
    rpi_fork(typing_thread, NULL);

    rpi_thread_start();
}