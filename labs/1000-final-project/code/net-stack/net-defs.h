#ifndef __NET_DEFS_H__
#define __NET_DEFS_H__

#include <stdint.h>

const static uint8_t ETH_BROADCAST[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // FF:FF:FF:FF:FF:FF
const static uint8_t IPV4_BROADCAST[4] = {255, 255, 255, 255}; // 255.255.255.255

/* ---------- Length Constants ---------- */
enum {
    FRAME_HEADER_BYTES = 14, // 6 bytes dest hw addr, 6 bytes src hw addr, 2 bytes length/length
    FRAME_MAX_PAYLOAD_SIZE = 420, // Just doing this for now (usually 1500)
    FRAME_MAX_SIZE = FRAME_HEADER_BYTES + FRAME_MAX_PAYLOAD_SIZE,
    FRAME_MIN_SIZE = 64,

    IPV4_MAX_SIZE = FRAME_MAX_PAYLOAD_SIZE,
    IPV4_PACKET_HEADER_WORDS = 5, // Assumes no options/padding at end
    IPV4_PACKET_HEADER_BYTES = IPV4_PACKET_HEADER_WORDS * 4,
    IPV4_MAX_DATA_SIZE = IPV4_MAX_SIZE - IPV4_PACKET_HEADER_BYTES,

    ICMP_MAX_SIZE = IPV4_MAX_DATA_SIZE,
    ICMP_HEADER_BYTES = 8,
    ICMP_MAX_DATA_SIZE = ICMP_MAX_SIZE - ICMP_HEADER_BYTES,
};

/* ---------- Frame types  ---------- */
enum { // https://www.iana.org/assignments/ieee-802-numbers/ieee-802-numbers.xhtml
    FRAME_IPV4                              = 0x0800,
    FRAME_ARP                               = 0x0806,
    FRAME_IPV6                              = 0x86DD,
    FRAME_LOCAL_EXPERIMENTAL_ETHERTYPE      = 0x88B5,
};

/* ---------- IPV4 Constants  ---------- */
enum { 

    // https://datatracker.ietf.org/doc/html/rfc1340
    // https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml

    N_A = 0,
    IPV4_VERS_4 = 4,

    PROTOCOL_ICMP = 1,
    PROTOCOL_TCP = 6,
    PROTOCOL_UDP = 17,
};

/* ---------- ICMP Constants  ---------- */
enum { // https://www.rfc-editor.org/rfc/rfc792
    ICMP_ECHO_MSG = 8,
    ICMP_ECHO_REPLY = 0,
};

typedef struct { // https://datatracker.ietf.org/doc/html/rfc9542
    uint8_t dest_hw_addr[6];
    uint8_t src_hw_addr[6];
    uint16_t ethertype; // Length. Going to make it FRAME_MAX_PAYLOAD_SIZE bytes or less
    uint8_t data[FRAME_MAX_PAYLOAD_SIZE];
} frame_t;
_Static_assert(sizeof(frame_t) == FRAME_MAX_SIZE, "frame_t size wrong");

typedef struct { // https://www.rfc-editor.org/rfc/rfc791.txt
    uint8_t version:4, 
            header_length_words:4;
    uint8_t type_of_service;
    uint16_t total_length;
    uint16_t identification;
    uint16_t flags:3, 
            fragment_offset:13;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint8_t src_ip_address[4];
    uint8_t dest_ip_address[4];
    // uint32_t options:24, 
    //          padding:8;
    uint8_t data[IPV4_MAX_DATA_SIZE];
    
} ipv4_t;
_Static_assert(sizeof(ipv4_t) == IPV4_MAX_SIZE, "ipv4_t size wrong");

typedef struct { // https://www.rfc-editor.org/rfc/rfc792
    uint8_t type; // 8 or 0
    uint8_t code;
    uint16_t checksum;
    uint16_t identifier;
    uint16_t seq_number;
    uint8_t data[ICMP_MAX_DATA_SIZE];
} icmp_echo_t;

_Static_assert(sizeof(icmp_echo_t) == ICMP_MAX_SIZE, "icmp_t size wrong");

#endif