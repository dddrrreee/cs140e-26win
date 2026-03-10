#include "rpi.h"
#include "spi.h"
#include "../print-utilities.h"
#include "../crc-16.h"
#include "../net-stack/netif/w5500.h"
#include "../net-stack/inet.h"

void print_ethertype(uint16_t ethertype) {
    if (ethertype <= 0x05DC) {
        trace("Packet length of %x\n", ethertype);
        return;
    }
    switch(ethertype) {
        case FRAME_IPV4:
            trace("FRAME_IPV4\n");
            break;
        case FRAME_IPV6:
            trace("FRAME_IPV6\n");
            break;
        case FRAME_ARP:
            trace("FRAME_ARP\n");
            break;
        case FRAME_LOCAL_EXPERIMENTAL_ETHERTYPE:
            trace("FRAME_LOCAL_EXPERIMENTAL_ETHERTYPE\n");
            break;
        default:
            // trace("Other ethertype: %x\n", ethertype);
            
    }
}

void print_protocol(uint16_t protocol) {
    switch(protocol) {
        case PROTOCOL_ICMP:
            trace("ICMP\n");
            break;
        case PROTOCOL_TCP:
            trace("TCP\n");
            break;
        case PROTOCOL_UDP:
            trace("UDP\n");
            break;
        default:
            trace("Other protocol: %x\n", protocol);
    }
}

int process_packets(const w5500_t* nic, uint8_t* rx_buffer, uint8_t socket) {
    
    int err;

    // ---------- Get frame ----------
    frame_t frame;
    uint16_t nbytes;
    err = inet_read_frame(&frame, &nbytes, socket);
    if (err != INET_SUCCESS) {
        // trace("Error reading frame: %d\n", err);
        return err;
    }
    print_ethertype(frame.ethertype);

    if (frame.ethertype != FRAME_IPV4) {
        w5500_fast_flush_rx(nic, socket);
        return 0;
    }

    // ---------- IPV4 Packet ----------

    uint8_t version = frame.data[0] >> 4;
    uint8_t ihl = (frame.data[0] & 0x0F) * 4;

    if (version != 4) {
        trace("Not IPv4\n");
        w5500_fast_flush_rx(nic, socket);
        return 0;
    }

    if (ihl < 20) {
        trace("Invalid IHL: %d\n", ihl);
        w5500_fast_flush_rx(nic, socket);
        return 0;
    }

    if (ihl != 20) {
        trace("IPv4 options unsupported (IHL=%d)\n", ihl);
        w5500_fast_flush_rx(nic, socket);
        return 0;
    }

    ipv4_t packet;
    uint16_t packet_length = nbytes - FRAME_HEADER_BYTES;
    memcpy(&packet, frame.data, packet_length);

    print_protocol(packet.protocol);

    uint16_t total_length = (frame.data[2] << 8) | frame.data[3]; // Received total length field

    if (total_length < ihl || total_length > packet_length) {
        trace("Invalid IPv4 length: total=%u ihl=%u packet=%u\n",
            total_length, ihl, packet_length);
        w5500_fast_flush_rx(nic, socket);
        return 0;
    }

    if (packet.protocol != PROTOCOL_ICMP) {
        trace("Skipping non-ICMP IPv4 packet (proto: %x)\n", packet.protocol);
        w5500_fast_flush_rx(nic, socket);
        return 0;
    }

    // ---------- ICMP Echo ----------
    if ((packet.data[0] != ICMP_ECHO_MSG) && (packet.data[0] != ICMP_ECHO_REPLY)) {
        trace("Skipping non-echo ICMP packet (type: %x)\n", packet.data[0]);
        w5500_fast_flush_rx(nic, socket);
        return 0;
    }
    icmp_echo_t echo;
    uint16_t echo_length = packet_length - IPV4_PACKET_HEADER_BYTES;
    memcpy(&echo, packet.data, echo_length);

    uint16_t data_len = echo_length - ICMP_HEADER_BYTES;
    print_bytes("RX: ", echo.data, data_len);
    print_as_string("RX: ", echo.data, data_len);



    // ---------- Flush the buffer after processing one packet ---------- (ik it's bad)
    w5500_fast_flush_rx(nic, socket);

    return nbytes;
}

#if 0
int process_packets(const w5500_t* nic, uint8_t* rx_buffer, uint8_t socket) {

    // Only if there are bytes are available
    uint16_t bytes_available = w5500_rx_available(nic, socket);
    if (bytes_available == 0)
        return 0;

    // Attempt to read bytes
    uint16_t read_bytes = w5500_read_rx_bytes(nic, rx_buffer, W5500_SOCKET_0);

    // Filter frame length
    if (read_bytes < FRAME_MIN_SIZE) {
        
        // trace("Frame length (%d) is shorter than %d\n", read_bytes, FRAME_MIN_SIZE);
        return 0;
    }
    if (read_bytes >= FRAME_MAX_SIZE) {
        trace("Frame length (%d) is longer than %d\n", read_bytes, FRAME_MAX_SIZE);
        return 0;
    }
    trace("1\n");

    // Filter frame type
    uint16_t ethertype = (rx_buffer[12] << 8) | rx_buffer[13];
    print_ethertype(ethertype);

    if (ethertype == FRAME_IPV6) {  // IPv4 + min IP header
        print_bytes("IPV6!: ", rx_buffer, read_bytes);
        print_as_string("IPV6!: ", rx_buffer, read_bytes);
        trace("\n");
        return 0;
    }
    trace("2\n");
    
    if (ethertype != FRAME_IPV4) {  // IPv4 + min IP header
        return 0;
    }
    uint8_t ip_protocol = rx_buffer[23];
    trace("3\n");
    print_protocol(ip_protocol);

    if (ip_protocol == PROTOCOL_UDP) {  // IPv4 + min IP header
        print_bytes("UDP!: ", rx_buffer, read_bytes);
        print_as_string("UDP!: ", rx_buffer, read_bytes);
        trace("\n");
        return 0;
    }
    if (ip_protocol != PROTOCOL_ICMP) {  // ICMP
        trace("Skipping non-ICMP IPv4 packet (proto: %x)\n", ip_protocol);
        return 0;
    }
    trace("4\n");
    print_bytes("RX: ", rx_buffer, read_bytes);
    print_as_string("RX: ", rx_buffer, read_bytes);
    trace("5\n");
    return read_bytes;
}
#endif

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

        inet_send_ping(IPV4_BROADCAST, message, msg_len, W5500_SOCKET_0);
        int status = process_packets(&nic, rx_buffer, W5500_SOCKET_0);

        // uint16_t rx_bytes = w5500_rx_available(&nic, W5500_SOCKET_0);
        delay_ms(10);
        // trace("RX buffer available: %d bytes\n", rx_bytes);

    }
    
}
