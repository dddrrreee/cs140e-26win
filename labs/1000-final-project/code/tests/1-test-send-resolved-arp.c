// with tests/arp_test.py

#include "rpi.h"
#include "spi.h"
#include "../net-stack/inet.h"
#include "../net-stack/arp.h"
#include "test-setup.h"

uint8_t PYTHON_SCRIPT_IP[4] = {1,2,3,4};
uint8_t PYTHON_SCRIPT_MAC[6] = {0x00,0x11,0x22,0x33,0x44,0x55};

void notmain(void) { 

    w5500_conf_t config = get_test_w5500_config();
    w5500_t nic;

    w5500_init(&nic, &config);

    verbose_t v = inet_verbosity_init();
    v.arp = 1;
    v.icmp = 1;
    v.arp = 1;
    v.data_link_send = 1;
    inet_init(&nic, &v);

    uint32_t entry_index = ARP_TABLE_SIZE;
    trace("Polling for frames...\n");

    uint8_t mac_buf[6];

    // Searching for ARP packet in order to fill the ARP table
    while(1) {

        inet_poll_frame(1);
        int err = inet_resolve_ip_address(PYTHON_SCRIPT_IP, mac_buf);

        if (err == INET_SUCCESS) {
            break;
        }

        if (err == INET_ARP_FOUND_BUT_INVALID) {
            trace("ARP entry seen but invalid, waiting...\n");
        }
        delay_ms(10);
    }

    // Correct IP was added to table
    trace("ARP entry discovered:\n");
    trace_arp_table("ARP Table now!");

    // Sending packet back to that ip ()
    const char* r = "rpi";
    inet_send_ping(PYTHON_SCRIPT_IP, ICMP_ECHO_REQUEST, r, strlen(r));

    while(1) {

        int err = inet_poll_frame(1);
        if (err > 0) {
            trace("Return code: %d\n", err);
            break;
        }
        if (err == ICMP_ECHO_REPLY) {
            trace("Ping handled, exiting loop\n");
            break;
        }

        delay_ms(10);
    }
}
