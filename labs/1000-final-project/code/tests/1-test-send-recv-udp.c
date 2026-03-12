#include "rpi.h"
#include "spi.h"
#include "../net-stack/netif/w5500.h"
#include "../net-stack/inet.h"

#include "test-setup.h"

static int return_from_udp = 0;
static const uint8_t dest_ip[4] = {1,2,3,4};
static uint16_t src_port = 8080;
static uint16_t dest_port = 24087;

void port_handler(const uint8_t* src_ip, uint16_t src_port, uint16_t dest_port,
    const uint8_t* data, uint16_t len) {
    
    trace("port_handler() called\n");
    const char* msg = "Hello from UDP handler!!!!";
    inet_udp_send(src_port, dest_port, dest_ip, msg, strlen(msg));
    return_from_udp = 1;
}

void notmain(void) { 

    int verbose_p = 0b0000; // Layer 3 debug

    w5500_conf_t config = get_test_w5500_config();
    w5500_t nic;
    w5500_init(&nic, &config);

    verbose_t v = inet_verbosity_init();
    v.arp = 1;
    // v.icmp = 1;
    v.udp = 1;
    // v.data_link_send = 1;
    inet_init(&nic, &v); // 0 IS THE ONE CLOSEST TO IMU

    
    
    // inet_send_arp() // TODO: send arp, wait for ip to be in the table, then send udp and see if it goes instead of just broadcasting
    const uint8_t eth_bridge[6] = {0xf8,0xe4,0x3b,0x51,0xce,0x67};
    // inet_add_arp_entry(dest_ip, eth_bridge);
    // trace_arp_table("Table:");

    // inet_udp_send(src_port, dest_port, dest_ip, msg, strlen(msg));

    inet_udp_add_port_handler(src_port, port_handler);

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
    
    while(1) {
        
        inet_poll_frame(0);
        

        if (return_from_udp)
            break;
    }

    trace("got UDP packet!");
    
}
