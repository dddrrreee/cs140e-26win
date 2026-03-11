#include "rpi.h"
#include "../net-stack/inet.h"
#include "../net-stack/arp.h"
#include "test-setup.h"

const uint8_t ip_addr[4] = {1,2,3,0};
const uint8_t hw_addr[6] = {1,2,3,4,5,6};

void notmain(void) { 
    w5500_conf_t config = get_test_w5500_config();

    trace_arp_table("Initial ARP Table:");
    arp_should_be_missing_from_table_test(ip_addr, hw_addr);
    trace_arp_table("ARP Table after calling 'find_arp_entry()':");
    arp_add_entry_test(ip_addr, hw_addr);
    trace_arp_table("ARP Table after calling 'inet_add_arp_entry()':");
    arp_entry_should_exist_test(ip_addr, hw_addr);
    trace_arp_table("ARP Table after calling 'find_arp_entry()' again:");
}
