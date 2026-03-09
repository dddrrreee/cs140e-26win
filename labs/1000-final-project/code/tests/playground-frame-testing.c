#include "rpi.h"
#include "spi.h"
#include "../print-utilities.h"
#include "../crc-16.h"
#include "../net-defs.h"
#include "../endian.h"
#include "../w5500.h"


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
    

    uint8_t* data_ptr;
    uint16_t checksum;
    

    const char* message = "Hello World! This is a message that needs to be long enough to send";
    uint16_t msg_len = strlen(message);

    // icmp_t icmp = {
    //     .echo = 1,
    //     .checksum = 0, // Checksum to be filled
    //     .identifier = N_A,
    //     .seq_number = N_A,
    //     .data_length = msg_len,
    //     .total_length = ICMP_HEADER_BYTES + msg_len
    // };

    // data_ptr = (uint8_t*)&icmp;
    // icmp.echo = !!icmp.echo * 8; // Make it 0 or 8
    // memcpy(icmp.data, message, icmp.data_length);
    // swapEndian16(&icmp.identifier);
    // swapEndian16(&icmp.seq_number);
    // checksum = our_crc16(data_ptr, icmp.total_length);
    // data_ptr[2] = ( (checksum >> 8) & 0xFF);
    // data_ptr[3] = (checksum & 0xFF);

    w5500_init(&nic, &config); // 0 IS THE ONE CLOSEST TO IMU
    
    while(1) {
        w5500_send_ping(&nic, IPV4_BROADCAST, message, msg_len, W5500_SOCKET_0);
        // w5500_write_broadcast_ipv4_packet(&nic, PROTOCOL_ICMP, &icmp, icmp.total_length, W5500_SOCKET_0);
        delay_ms(1000);
    }
    
}
