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
    v.data_link_send = 1;
    inet_init(&nic, &v);

    uint32_t entry_index = ARP_TABLE_SIZE;
    trace("Polling for frames...\n");

    uint8_t mac_buf[6];

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

    // Resolves to correct MAC address
    // if (memcmp(mac_buf, PYTHON_SCRIPT_MAC, MAC_ADDR_LENGTH) != 0 ) {
    //     trace("IP address was found in table but not resolved to correct MAC\n");
    //     panic("Expected: {0x%X:0x%X:0x%X:0x%X:0x%X:0x%X}",
    //         mac_buf[0], mac_buf[1], mac_buf[2], mac_buf[3], mac_buf[4], mac_buf[5]);
    // }

    // Sending packet back to that ip ()
    const char* r = "rpi";
    inet_send_ping(PYTHON_SCRIPT_IP, ICMP_ECHO_REQUEST, r, strlen(r));

    while(1) {

        int err = inet_poll_frame(1);
        if (err > 0) {
            trace("Return code: %d\n", err);
            // inet_send_broadcast_frame(FRAME_LOCAL_EXPERIMENTAL_ETHERTYPE, r, strlen(r));
            break;
        }
        if (err == ICMP_ECHO_REPLY) {
            trace("Ping handled, exiting loop\n");
            break;
        }

    //     int err = inet_resolve_ip_address(PYTHON_SCRIPT_IP, mac_buf);

    //     if (err == INET_SUCCESS) {

    //         break;
    //     }

    //     if (err == INET_ARP_FOUND_BUT_INVALID) {
    //         trace("ARP entry seen but invalid, waiting...\n");
    //     }
        delay_ms(10);
    }

    // TODO: finish the verbose pinging 
}
