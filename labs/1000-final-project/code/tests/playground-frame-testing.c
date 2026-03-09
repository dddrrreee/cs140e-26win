#include "rpi.h"
#include "spi.h"
#include "../print-utilities.h"
#include "../crc-16.h"
#include "../net-defs.h"
#include "../w5500.h"

void write_macraw_bytes(const w5500_t* nic, const frame_t* frame) {
    while(1) {
        trace("H\n");
        w5500_write_frame(nic, frame, W5500_SOCKET_0);
        delay_ms(1000);
    }
}

static inline void swapNibbles8(void* val) {
    uint8_t* u8 = val;
    *u8 = ((*u8 & 0xF0) >> 4) | 
           ((*u8 & 0x0F) << 4);
}

static inline void swapEndian16(void* val) {
    uint16_t* u16 = val;
    *u16 = ((*u16 & 0xFF00) >> 8) | 
           ((*u16 & 0x00FF) << 8);
}

static inline void swapEndian32(void* val) {
    uint32_t* u32 = val;
    *u32 = ((*u32 & 0xFF000000) >> 24) | 
           ((*u32 & 0x00FF0000) >> 8) | 
           ((*u32 & 0x0000FF00) << 8) | 
           ((*u32 & 0x000000FF) << 24);
}



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

    icmp_t icmp = {
        .echo = 1,
        .checksum = 0, // Checksum to be filled
        .identifier = N_A,
        .seq_number = N_A,
        .data_length = msg_len,
        .total_length = ICMP_HEADER_BYTES + msg_len
    };

    data_ptr = (uint8_t*)&icmp;
    icmp.echo = !!icmp.echo * 8; // Make it 0 or 8
    memcpy(icmp.data, message, icmp.data_length);
    swapEndian16(&icmp.identifier);
    swapEndian16(&icmp.seq_number);
    checksum = our_crc16(data_ptr, icmp.total_length);
    data_ptr[2] = ( (checksum >> 8) & 0xFF);
    data_ptr[3] = (checksum & 0xFF);

    uint16_t packet_actual_len = icmp.total_length + IPV4_PACKET_HEADER_BYTES;
    ipv4_t packet = {
        .version = IPV4_VERS_4,
        .header_length_words = IPV4_PACKET_HEADER_WORDS,
        .type_of_service = N_A,
        .total_length = packet_actual_len,
        .identification = N_A,
        .flags = N_A,
        .fragment_offset = N_A,
        .ttl = 1,
        .protocol = PROTOCOL_ICMP,
        // Checksum to be filled in
        // Src addr to be filled in
        .dest_ip_address = IP_BROADCAST,
        // Payload to be filled in
        .data_length = icmp.total_length
    };
    data_ptr = (uint8_t*)&packet;
    memcpy(packet.src_ip_address, config.ipv4_addr, 4);
    memcpy(packet.data, &icmp, icmp.total_length);

    swapNibbles8(&packet);
    swapEndian16(&packet.total_length);
    
    checksum = our_crc16(data_ptr, packet_actual_len);
    data_ptr[10] = ( (checksum >> 8) & 0xFF);
    data_ptr[11] = (checksum & 0xFF);

//       uint16_t checksum = calc_checksum(payload);
  
    // data_ptr[1] = ((packet.version << 4) & 0xF0) | (packet.header_length_words & 0xF);

    frame_t frame = {
        .dest_hw_addr = ETH_BROADCAST,
        .ethertype = FRAME_IPV4,
        .data_length = packet_actual_len,
        .total_length = packet_actual_len + FRAME_HEADER_BYTES
    };
    data_ptr = (uint8_t*)&frame;
    memcpy(frame.src_hw_addr, config.hw_addr, 6); // Needs to go in an init
    memcpy(frame.data, &packet, packet_actual_len);
    swapEndian16(&frame.ethertype); // Swap ethertype

    print_bytes("Frame:", &frame, frame.total_length);

    // return;

    w5500_init(&nic, &config); // 0 IS THE ONE CLOSEST TO IMU
    
    while(1) {
        w5500_write_frame(&nic, &frame, W5500_SOCKET_0);
        delay_ms(1000);
        // break;
    }
    
}
