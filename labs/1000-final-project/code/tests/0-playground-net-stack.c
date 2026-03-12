#include "rpi.h"
#include "spi.h"
#include "../print-utilities.h"
#include "../crc-16.h"
#include "../net-stack/netif/w5500.h"
#include "../net-stack/inet.h"
#include "../net-stack/data-link.h"

#include "test-setup.h"

void notmain(void) { 

    w5500_conf_t config = get_test_w5500_config();
    w5500_t nic;
    w5500_init(&nic, &config);

    verbose_t v = inet_verbosity_init();
    // v.arp = 1;
    // v.icmp = 1;
    // v.ipv4 = 1;
    v.udp = 1;
    v.data_link_send = 1;
    inet_init(&nic, &v); // 0 IS THE ONE CLOSEST TO IMU

    while(1) {

        uint8_t flush_buffer = 1;
        inet_poll_frame(flush_buffer);

        delay_ms(10);

    }
    
}
