#include "rpi.h"
#include "spi.h"
#include "../print-utilities.h"
#include "../crc-16.h"
#include "../net-stack/netif/w5500.h"
#include "../net-stack/inet.h"
// #include "../w5500.h"


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
    

    inet_nic_init(&nic); // 0 IS THE ONE CLOSEST TO IMU

    const char* message = "Hello World! This is a message that needs to be long enough to send";
    uint16_t msg_len = strlen(message);



    while(1) {

        trace("RX buffer available: %d bytes\n", w5500_rx_available(&nic, W5500_SOCKET_0));
        w5500_fast_flush_rx(&nic, W5500_SOCKET_0);

        inet_send_ping(IPV4_BROADCAST, message, msg_len, W5500_SOCKET_0);
        // w5500_write_broadcast_ipv4_packet(&nic, PROTOCOL_ICMP, &icmp, icmp.total_length, W5500_SOCKET_0);
        delay_ms(1000);
    }
    
}
