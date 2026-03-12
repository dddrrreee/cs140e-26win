// with tests/1-test-fill-arp-table.py

#include "rpi.h"
#include "spi.h"
#include "../net-stack/inet.h"
#include "../net-stack/arp.h" // For printing

uint8_t target_ip[4] = {1,2,3,3};
uint8_t hw_addr[6] = {0,0,0,0,0,0};

void notmain(void) { 

    w5500_conf_t config = {
        .chip_select = 0,
        .clk_div = 80,
        .hw_addr = {0x76, 0x67, 0x67, 0x67, 0x67, 0x67},
        .ipv4_addr = {192, 168, 0, 3},
        .gateway_addr = {192, 168, 0, 1},
        .subnet_mask = {255, 255, 255, 0},
        // .sockets_enabled = 0b00000001,
        .phy_mode = W5500_ALL_CAPABLE_AUTO_NEG_EN,
    };
    w5500_t nic;
    
    w5500_init(&nic, &config);
    inet_init(&nic, 0);

    trace("Polling for frames...\n");

    while(1) {

        inet_poll_frame(1);

        int err = inet_resolve_ip_address(target_ip, hw_addr);

        if (err == INET_SUCCESS) {
            trace("ARP entry discovered:\n");
            print_arp_entry(target_ip, hw_addr, 1);
            trace("\n");
            break;
        }

        if (err == INET_ARP_FOUND_BUT_INVALID) {
            trace("ARP entry seen but invalid, waiting...\n");
        }
        delay_ms(10);
    }

    trace_arp_table("ARP Table now!");
    
    printk("PASS: %s\n", __FILE__);
}
