#include "rpi.h"
#include "spi.h"
#include "../print-utilities.h"
#include "../crc-16.h"
#include "../net-stack/netif/w5500.h"
#include "../net-stack/inet.h"
#include "../net-stack/data-link.h"

void notmain(void) { 

    int verbose_p = 0b000; // Layer 3 debug

    w5500_conf_t config = {
        .chip_select = 0,
        .clk_div = 80,
        .hw_addr = {0x76, 0x67, 0x67, 0x67, 0x67, 0x67},
        .ipv4_addr = {192, 168, 0, 3},
        .gateway_addr = {192, 168, 0, 1},
        .subnet_mask = {255, 255, 255, 0},
        .phy_mode = W5500_ALL_CAPABLE_AUTO_NEG_EN,
    };
    w5500_t nic;
    w5500_init(&nic, &config);

    inet_init(&nic, verbose_p); // 0 IS THE ONE CLOSEST TO IMU

    uint8_t rx_buffer[FRAME_MAX_SIZE];

    const char* message = "Hello World! This is a message that needs to be long enough to send";
    uint16_t msg_len = strlen(message);

    while(1) {

        // uint8_t ip[4] = {1,1,1,1};
        // inet_send_ping(IPV4_BROADCAST, message, msg_len, W5500_SOCKET_0);

        uint8_t flush_buffer = 1;
        inet_poll_frame(flush_buffer);

        // delay_ms(10);

    }
    
}
