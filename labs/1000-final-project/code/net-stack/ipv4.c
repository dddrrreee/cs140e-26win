/*
LAYER 3: IPV4
*/ 

#include "inet.h"

#include "../endian.h"
#include "../crc-16.h"
#include "../print-utilities.h"

#include "ipv4.h"

#include "icmp.h"
#include "udp.h"

static ipv4_t _ipv4_rx; // IPv4 buffer. will do one for each protocol perhaps

static int _verbose_p = 0;
static int _verbose_full_p = 0;

/**********************************************************
 * Setup
 */

void ipv4_init(const verbose_t* verbosity) {
    if (verbosity->ipv4 || verbosity->all) {
        _verbose_p = 1;
        trace("IPV4 layer verbosity enabled\n");
    }

    if (_verbose_full_p) {
        _verbose_full_p = 1;
        trace("IPV4 layer full packet verbosity enabled\n");
    }
}

/**********************************************************
 * Public interface
 */

int inet_send_ipv4_packet(const uint8_t* dest_ipv4_addr, uint8_t ipv4_protocol, const void* data, uint16_t nbytes) {

    // ---------- TX packet ---------- 
    uint16_t packet_length = nbytes + IPV4_PACKET_HEADER_BYTES;
    ipv4_t packet;
    packet.version = IPV4_VERS_4; // Gonna assume we aren't changing this
    packet.header_length_words = IPV4_PACKET_HEADER_WORDS;
    packet.type_of_service = N_A;
    packet.total_length = packet_length;
    packet.identification = N_A;
    packet.flags = N_A;
    packet.fragment_offset = N_A;
    packet.ttl = IPV4_DEFAULT_TTL;
    packet.protocol = ipv4_protocol;
    packet.checksum = 0; // Checksum to be filled later but must be 0 for now
    memcpy(packet.src_ipv4_address, inet_get_ipv4_addr(), IPV4_ADDR_BYTES);
    memcpy(packet.dest_ip_address, dest_ipv4_addr, IPV4_ADDR_BYTES);
    memcpy(packet.data, data, nbytes);

    // ---------- Packet conditioning for endianness and stuff ---------- 
    swapNibbles8(&packet);                  // Swap version / header length byte [0]
    swapEndian16(&packet.total_length);     // Swap total length (little->big endian) [2:3]

    // ---------- crc16 ---------- 
    uint8_t* cksum_ptr = (uint8_t*)&packet.checksum;
    uint16_t checksum = our_crc16(&packet, IPV4_PACKET_HEADER_BYTES);
    *cksum_ptr = ( (checksum >> 8) & 0xFF);
    *(cksum_ptr + 1) = (checksum & 0xFF);

    // TODO: resolve IP ADDRESS
    uint8_t dest_hw_addr[6];
    inet_resolve_ip_address(dest_ipv4_addr, dest_hw_addr);

    if (_verbose_p)
        trace("Sending IPV4 packet of type %d with payload length %d to {%d.%d.%d.%d}\n",
            ipv4_protocol, packet_length,
            dest_ipv4_addr[0], dest_ipv4_addr[1],
            dest_ipv4_addr[2], dest_ipv4_addr[3]);
    
    // ---------- Make frame (Layer 2)! ---------- 
    return inet_send_frame(dest_hw_addr, FRAME_IPV4, &packet, packet_length);
}

int inet_ipv4_handler(const uint8_t* data, uint16_t packet_bytes) {
    int err;
    memcpy(&_ipv4_rx, data, packet_bytes);

    // 1. Checksum TODO
    // uint16_t recv_cksum = ipv4->checksum;
    // swapEndian16(&ipv4->total_length);

    // 2. Flipping endians
    swapNibbles8(&_ipv4_rx);                  // Swap version / header length byte [0]
    swapEndian16(&_ipv4_rx.total_length);     // Swap total length (little->big endian) [2:3]

    // TODO: more checks?


    if (_verbose_full_p) {
        print_bytes("Layer 3 ipv4: ", &_ipv4_rx, packet_bytes);
    }

    // 3. Check header
    err = ipv4_check_header(&_ipv4_rx, packet_bytes);
    if (err != INET_SUCCESS) {
        return err;
    }

    // 4. ipv4_addr checks

    return ipv4_protocol_handler(&_ipv4_rx, packet_bytes);
}


/**********************************************************
 * Helpers for ipv4
 */

int ipv4_check_header(const ipv4_t* packet, uint16_t header_bytes) {

    // Make sure it is IPV4!!!!!
    if (packet->version != IPV4_VERS_4) {
        return INET_NOT_IPV4;
    }

    if (packet->header_length_words != IPV4_PACKET_HEADER_WORDS) {
        if (_verbose_p)
            trace("INET_IPV4_UNSUPPORTED_HEADER_LEN: %d, expected %d\n", packet->header_length_words, IPV4_PACKET_HEADER_WORDS);
        return INET_IPV4_UNSUPPORTED_HEADER_LEN;
    }

    if (header_bytes < ICMP_HEADER_BYTES || header_bytes > IPV4_MAX_SIZE) {
        if (_verbose_p)
            trace("Invalid IPv4 length: total=%u header_len=%u max=%u\n",
            header_bytes, ICMP_HEADER_BYTES, IPV4_MAX_SIZE);
        return INET_ERROR;
    }

    return INET_SUCCESS;
}


// https://datatracker.ietf.org/doc/html/rfc1340
// https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml
int ipv4_protocol_handler(const ipv4_t* packet, uint16_t packet_bytes)  {
    int err;
    uint8_t protocol = packet->protocol;

    switch(protocol) {
        case PROTOCOL_ICMP:
            return inet_icmp_handler(packet->data, packet->src_ipv4_address, packet_bytes - IPV4_PACKET_HEADER_BYTES);
            
        case PROTOCOL_UDP:

            return inet_udp_handler(packet->data, packet->src_ipv4_address, packet_bytes - IPV4_PACKET_HEADER_BYTES);
        default:
            if (_verbose_p)
                trace("Unhandled IPV4 Protocol %d\n", protocol);
            return INET_IPV4_UNSUPPORTED_PROTOCOL;
    }

    if (_verbose_p)
        trace("IPv4 Not Handled quite yet\n");
    return INET_SUCCESS;
}

