#include "rpi.h"
#include "spi.h"
#include "../print-utilities.h"
#include "../crc-16.h"
#include "../net-stack/netif/w5500.h"
#include "../net-stack/inet.h"


void notmain(void) { 

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
    

    inet_nic_init(&nic); // 0 IS THE ONE CLOSEST TO IMU

    uint8_t rx_buffer[FRAME_MAX_SIZE];

    const char* message = "Hello World! This is a message that needs to be long enough to send";
    uint16_t msg_len = strlen(message);

    while(1) {

        // Send packet for a ping
        inet_send_ping(IPV4_BROADCAST, message, msg_len, W5500_SOCKET_0);

        uint16_t rx_bytes = w5500_rx_available(&nic, W5500_SOCKET_0);
        trace("RX buffer available: %d bytes\n", rx_bytes);

        if (rx_bytes > 0) {
            uint16_t read_bytes = w5500_read_rx_bytes(&nic, rx_buffer, W5500_SOCKET_0);
            if (read_bytes > 0) {
                print_bytes("RX: ", rx_buffer, read_bytes);
            }
        }

        delay_ms(100);
    }
    
}
