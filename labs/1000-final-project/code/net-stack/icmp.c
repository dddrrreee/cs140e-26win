// LAYER 2: Ethernet Frame Handling with network interface (W5500 driver)

#include "inet.h"

#include "../crc-16.h"

int inet_icmp_handler(const uint8_t* data, const uint8_t* src_addr, uint16_t icmp_bytes) {
    
    // https://www.rfc-editor.org/rfc/rfc792
    uint8_t icmp_type = data[0];

    switch (icmp_type) {
        case ICMP_ECHO_MSG:
            // trace("ICMP ECHO!\n");
            const char* msg = "THIS IS A REPLY";
            // Send back to the source of the echo request
            inet_send_ping(src_addr, ICMP_ECHO_REPLY, msg, strlen(msg), W5500_SOCKET_0);
            return INET_SUCCESS;
        case ICMP_ECHO_REPLY:
            trace("ICMP REPLY!\n");
            return INET_SUCCESS;
        default:
            return INET_ICMP_UNSUPPORTED_TYPE;
    }

    return INET_SUCCESS;
}


uint16_t inet_send_ping(const uint8_t* dest_ipv4_addr, uint8_t ping_type, const void* data, uint16_t nbytes, uint8_t socket) {

    uint16_t icmp_length = nbytes + ICMP_HEADER_BYTES;

    if (ping_type != 0)
        ping_type = ICMP_ECHO_MSG;

    icmp_echo_t icmp;
    icmp.type = ping_type;
    icmp.code = 0;
    icmp.checksum = 0; // Checksum to be filled
    icmp.identifier = N_A; // Not used in icmp echo
    icmp.seq_number = N_A; // Not used in icmp echo
    memcpy(icmp.data, data, nbytes);

    uint8_t* cksum_ptr = (uint8_t*)&icmp.checksum;
    uint16_t checksum = our_crc16(&icmp, icmp_length);
    *cksum_ptr = ( (checksum >> 8) & 0xFF);
    *(cksum_ptr + 1) = (checksum & 0xFF);

    // print_bytes("ICMP packet: ", &icmp, icmp_length);

    return inet_write_ipv4_packet(dest_ipv4_addr, PROTOCOL_ICMP, &icmp, icmp_length, socket);
}

