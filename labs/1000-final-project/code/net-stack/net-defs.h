#ifndef __NET_DEFS_H__
#define __NET_DEFS_H__

#include <stdint.h>

#include "netif/w5500-defs.h" // ???

const static uint8_t MAC_BROADCAST[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // FF:FF:FF:FF:FF:FF
const static uint8_t IPV4_BROADCAST[4] = {255, 255, 255, 255}; // 255.255.255.255
#define INET_NIC_SOCKET W5500_SOCKET_0
//*******************************************************************************************************************
//                          Error Codes
//*******************************************************************************************************************

enum {
    INET_SUCCESS = 0,
    INET_ERROR = -1,
    INET_NOT_IMPLEMENTED = -2,

    INET_WRITE_FRAME_ERROR = -4,



    // Frame
    INET_MAC_FILTERED = -12,
    INET_NOT_IPV4 = -13,
    INET_MAC_NOT_FOR_US = -14,
    INET_NO_DATA_READ = -15,
    INET_NIC_NOT_INITIALIZED = -16,
    INET_FRAME_802_3 = -18,

    // ARP
    INET_ARP_RECEIVED = 20,
    INET_ARP_SENT = 21,
    INET_ARP_BAD_HTYPE = -20,
    INET_ARP_INVALID_IP_LEN = -21,
    INET_ARP_INVALID_MAC_LEN = -22,
    INET_ARP_INVALID_OP = -23,
    INET_ARP_NOT_FOR_US = -24,
    INET_ARP_INVALID_MSG_LEN = -25,
    INET_ARP_NO_TABLE_ENTRY = -26,
    INET_ARP_FOUND_BUT_INVALID = -26,
    INET_ARP_TABLE_FULL = -26,

    // IPV4
    INET_IPV4_UNSUPPORTED_VERS = -50,
    INET_IPV4_UNSUPPORTED_HEADER_LEN = -51,

    // Unsupported
    INET_FRAME_UNSUPPORTED_ETHERTYPE = -200,
    INET_IPV4_UNSUPPORTED_PROTOCOL = -201,
    INET_ICMP_UNSUPPORTED_TYPE = -202,
};

//*******************************************************************************************************************
//                          Lengths
//*******************************************************************************************************************

/* ---------- Length Constants ---------- */
enum {
    MAC_ADDR_LENGTH = 6,
    IPV4_ADDR_LENGTH = 4,

    // Frame
    FRAME_HEADER_BYTES = 14, // 6 bytes dest hw addr, 6 bytes src hw addr, 2 bytes length/length
    FRAME_MAX_PAYLOAD_SIZE = 420, // Just doing this for now (usually 1500)
    FRAME_MAX_SIZE = FRAME_HEADER_BYTES + FRAME_MAX_PAYLOAD_SIZE,
    FRAME_MIN_SIZE = 64,

    // ARP
    ARP_MESSAGE_BYTES = 28,
    ARP_TABLE_SIZE = 32,

    // IPV4
    IPV4_MAX_SIZE = FRAME_MAX_PAYLOAD_SIZE,
    IPV4_PACKET_HEADER_WORDS = 5, // Assumes no options/padding at end
    IPV4_PACKET_HEADER_BYTES = IPV4_PACKET_HEADER_WORDS * 4,
    IPV4_MAX_DATA_SIZE = IPV4_MAX_SIZE - IPV4_PACKET_HEADER_BYTES,

    ICMP_MAX_SIZE = IPV4_MAX_DATA_SIZE,
    ICMP_HEADER_BYTES = 8,
    ICMP_MAX_DATA_SIZE = ICMP_MAX_SIZE - ICMP_HEADER_BYTES,


};


//*******************************************************************************************************************
//                          ETHERTYPE
//*******************************************************************************************************************

/* ---------- Frame types  ---------- */
enum { //  https://www.cavebear.com/archive/cavebear/Ethernet/type.html
    FRAME_IEEE802_3                         = 0x05DC, // Not handled. If length field is <= this value, it is a IEEE 802.3 frame
    FRAME_IPV4                              = 0x0800,
    FRAME_ARP                               = 0x0806,
    FRAME_IPV6                              = 0x86DD,
    FRAME_LOCAL_EXPERIMENTAL_ETHERTYPE      = 0x88B5,
};

//*******************************************************************************************************************
//                          ARP
//*******************************************************************************************************************

enum { // https://www.rfc-editor.org/rfc/rfc826.html
    ARP_HTYPE_ETHERNET = 1,
    ARP_REQUEST = 1,
    ARP_REPLY = 2,
};

//*******************************************************************************************************************
//                          IPV4 and Protocol
//*******************************************************************************************************************

enum { 

    // https://datatracker.ietf.org/doc/html/rfc1340
    // https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml

    N_A = 0,
    IPV4_VERS_4 = 4,
    IPV4_DEFAULT_TTL = 64,

    PROTOCOL_ICMP = 1,
    PROTOCOL_TCP = 6,
    PROTOCOL_UDP = 17,
};

//*******************************************************************************************************************
//                          ICMP
//*******************************************************************************************************************

enum { // https://www.rfc-editor.org/rfc/rfc792
    ICMP_ECHO_MSG = 8,
    ICMP_ECHO_REPLY = 0,
};

//*******************************************************************************************************************
//                          frame_t
//*******************************************************************************************************************

typedef struct { // https://datatracker.ietf.org/doc/html/rfc9542
    uint8_t dest_hw_addr[6];
    uint8_t src_hw_addr[6];
    uint16_t ethertype; // Length. Going to make it FRAME_MAX_PAYLOAD_SIZE bytes or less
    uint8_t data[FRAME_MAX_PAYLOAD_SIZE];
} frame_t;
_Static_assert(sizeof(frame_t) == FRAME_MAX_SIZE, "frame_t size wrong");

//*******************************************************************************************************************
//                          ipv4_t
//*******************************************************************************************************************
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
    uint8_t src_ipv4_address[4];
    uint8_t dest_ip_address[4];
    // uint32_t options:24, 
    //          padding:8;
    uint8_t data[IPV4_MAX_DATA_SIZE];
    
} ipv4_t;
_Static_assert(sizeof(ipv4_t) == IPV4_MAX_SIZE, "ipv4_t size wrong");

//*******************************************************************************************************************
//                          icmp
//*******************************************************************************************************************
typedef struct { // https://www.rfc-editor.org/rfc/rfc792
    uint8_t type; // 8 or 0
    uint8_t code;
    uint16_t checksum;
    uint16_t identifier;
    uint16_t seq_number;
    uint8_t data[ICMP_MAX_DATA_SIZE];
} icmp_echo_t;
_Static_assert(sizeof(icmp_echo_t) == ICMP_MAX_SIZE, "icmp_t size wrong");

//*******************************************************************************************************************
//                          arp
//*******************************************************************************************************************

typedef struct { // https://www.rfc-editor.org/rfc/rfc826.html
    uint16_t hardware_type; // 1 indicated Ethernet
    uint16_t protocol_type; // For IPV4, same as protocol (0x0800)
    uint8_t hardware_len; // Length of hw_addr (6 bytes)
    uint8_t protocol_len; // Length of ip_addr (6 bytes)
    uint16_t operation; // 1 for request, 2 for reply

    uint8_t src_hw_addr[6];
    uint8_t src_ipv4_addr[4];
    uint8_t dest_hw_addr[6];
    uint8_t dest_ipv4_addr[4];
} arp_packet_t;
_Static_assert(sizeof(arp_packet_t) == 28, "arp_packet_t size wrong");

typedef struct {
    uint8_t ip_addr[4];    // 4-byte IP address (mapped key)
    uint8_t hw_addr[6];    // 6-byte MAC address (mapped value)
    uint8_t valid;
    // uint16_t valid:1,       
    //          dynamic:1,     // Dynamic or static
    //          timeout_s:14;  // IF dynamic
    // uint16_t epoch;         // IF dynamic
    // Other fields like timestamp for aging out dynamic entries could be added here
} arp_table_entry_t;
_Static_assert(sizeof(arp_table_entry_t) == 11, "arp_table_t size wrong");


#endif