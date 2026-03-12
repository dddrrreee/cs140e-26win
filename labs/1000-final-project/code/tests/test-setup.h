#ifndef __TEST_SETUP_H__
#define __TEST_SETUP_H__

#include "../net-stack/netif/w5500.h"
#include "../net-stack/inet.h"
#include "../net-stack/arp.h"

w5500_conf_t get_test_w5500_config() {
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
    return config;
}

// ---------- ARP ----------

void arp_should_be_missing_from_table_test(const uint8_t* ip_addr, const uint8_t* mac_addr) {
    uint8_t ip[IPV4_ADDR_BYTES];
    uint8_t mac[MAC_ADDR_BYTES];

    uint8_t zeros[6];
    memset(zeros, 0, 6);

    memcpy(ip, ip_addr, IPV4_ADDR_BYTES);
    memcpy(mac, mac_addr, MAC_ADDR_BYTES);

    // uint32_t entry_index;
    int err;
    for (int i = 0; i < 4; i++) {
        ip[3] = i;

        // Resolving by ip
        uint8_t mac_buf[MAC_ADDR_BYTES];
        memset(mac_buf, 0, MAC_ADDR_BYTES);

        err = inet_resolve_ip_address(ip, mac_buf);
        if (err != INET_ARP_NO_TABLE_ENTRY) // Want there to be nothing
            panic("Entry %d not found by ipv4 with error %d\n", i, err);
        if (memcmp(mac_buf, zeros, MAC_ADDR_BYTES) != 0) // Want mac_buf to be unchanged
            panic("Resolving by ipv4 set mac address%d\n", err);


        // Resolving by mac
        uint8_t ip_buf[IPV4_ADDR_BYTES];
        memset(ip_buf, 0, IPV4_ADDR_BYTES);

        err = inet_resolve_hw_address(mac, ip_buf);
        if (err != INET_ARP_NO_TABLE_ENTRY) // Want there to be nothing
            panic("Entry %d not found by mac with error %d\n", i, err);
        if (memcmp(ip_buf, zeros, IPV4_ADDR_BYTES) != 0) // Want mac_buf to be unchanged
            panic("Resolving by mac set ip address%d\n", err);
        
    }
}

void arp_add_entry_test(const uint8_t* ip_addr, const uint8_t* mac_addr) {
    uint8_t temp_ip[IPV4_ADDR_BYTES];
    uint8_t temp_mac[MAC_ADDR_BYTES];

    memcpy(temp_ip, ip_addr, IPV4_ADDR_BYTES);
    memcpy(temp_mac, mac_addr, MAC_ADDR_BYTES);

    for (int i = 0; i < 4; i++) {
        temp_ip[3] = i;
        temp_mac[5] = i;
        int err = inet_add_arp_entry(temp_ip, temp_mac);

        switch (err) {
            case INET_SUCCESS:
                trace("Found at entry %d\n", i);
                break;
            case INET_ARP_NO_TABLE_ENTRY:
                panic("Entry %d not found, supposed to be at index %d\n", i, i);
            case INET_ARP_FOUND_BUT_INVALID:
                panic("Entry %d found but invalid\n", i);
            default:
                panic("Unhandled error %d !?!?! \n", err);
        }
    }
}

void arp_entry_should_exist_test(const uint8_t* ip_addr, const uint8_t* mac_addr) {
    uint8_t ip[IPV4_ADDR_BYTES];
    uint8_t mac[MAC_ADDR_BYTES];

    memcpy(ip, ip_addr, IPV4_ADDR_BYTES);
    memcpy(mac, mac_addr, MAC_ADDR_BYTES);

    int err;
    for (int i = 0; i < 4; i++) {
        ip[3] = i;
        mac[5] = i;

        // Resolving by ip
        uint8_t mac_buf[MAC_ADDR_BYTES];
        memset(mac_buf, 0, MAC_ADDR_BYTES);
        err = inet_resolve_ip_address(ip, mac_buf);

        switch (err) {
            case INET_SUCCESS:
                trace("Entry %d Found!\n", i);
                break;
            case INET_ARP_NO_TABLE_ENTRY:
                panic("Entry %d not found, supposed to be at index %d\n", i, i);
            case INET_ARP_FOUND_BUT_INVALID:
                panic("Entry %d found but invalid\n", i);
            default:
                panic("Unhandled error %d !?!?! \n", err);
        }
        if (memcmp(mac_buf, mac, MAC_ADDR_BYTES) != 0) {// Want mac_buf to be changed
            trace("Resolving by ipv4 set mac address wrong. printing below\n");
            print_arp_entry(ip, mac_buf, 1);
            dev_barrier();
            panic("");
        }


        // Resolving by mac
        uint8_t ip_buf[IPV4_ADDR_BYTES];
        memset(ip_buf, 0, IPV4_ADDR_BYTES);
        err = inet_resolve_hw_address(mac, ip_buf);

        switch (err) {
            case INET_SUCCESS:
                trace("Entry %d Found!\n", i);
                break;
            case INET_ARP_NO_TABLE_ENTRY:
                panic("Entry %d not found, supposed to be at index %d\n", i, i);
            case INET_ARP_FOUND_BUT_INVALID:
                panic("Entry %d found but invalid\n", i);
            default:
                panic("Unhandled error %d !?!?! \n", err);
        }

        if (memcmp(ip_buf, ip, IPV4_ADDR_BYTES) != 0) { // Want ip_buf to be changed
            trace("Resolving by mac set ipv4 address wrong. printing below\n");
            print_arp_entry(ip_buf, mac, 1);
            panic("");
        }
        
    }
}

// void arp_entry_should_exist_test(const uint8_t* ip_addr, const uint8_t* mac_addr) {
//     uint8_t temp_ip[IPV4_ADDR_BYTES];
//     uint8_t temp_mac[MAC_ADDR_BYTES];

//     memcpy(temp_ip, ip_addr, IPV4_ADDR_BYTES);
//     memcpy(temp_mac, mac_addr, MAC_ADDR_BYTES);

//     uint32_t entry_index;

//     for (int i = 0; i < 4; i++) {
//         temp_ip[3] = i;
//         temp_mac[5] = i;
//         int err = find_arp_entry(temp_ip, &entry_index);
        
//         switch (err) {
//             case INET_SUCCESS:
//                 trace("Found at entry %d\n", i);
//                 break;
//             case INET_ARP_NO_TABLE_ENTRY:
//                 panic("Entry %d not found, supposed to be at index %d\n", i, i);
//             case INET_ARP_FOUND_BUT_INVALID:
//                 panic("Entry %d found but invalid\n", i);
//             default:
//                 panic("Unhandled error %d !?!?! \n", err);
//         }

//         // if ()
//     }
// }


#endif