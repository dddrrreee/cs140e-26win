/*
LAYER 3: Packet handling! 

IP layer and all the packet handling logic
- ARP and IPv4

*/ 

#include "inet.h"

#include "../endian.h"
#include "../crc-16.h"
#include "../print-utilities.h"

#include "network.h"

#include "icmp.h"

uint8_t _ipv4_addr[4]; // At this layer

static ipv4_t _ipv4_rx; // IPv4 buffer. will do one for each protocol perhaps

static int _verbose_p = 0;

/**********************************************************
 * Setup
 */

int inet_layer3_init(uint8_t* ipv4_addr, int verbose_p) {

    _verbose_p = (verbose_p >> 3) & 1;
    // Layer 3
    memcpy(_ipv4_addr, ipv4_addr, 4);
    
    return INET_SUCCESS;
}



/**********************************************************
 * Public interface
 */

uint16_t inet_write_ipv4_packet(const uint8_t* dest_ipv4_addr, uint8_t ipv4_protocol, const void* data, uint16_t nbytes, uint8_t socket) {

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
    packet.ttl = 1;
    packet.protocol = ipv4_protocol;
    packet.checksum = 0; // Checksum to be filled later but must be 0 for now
    memcpy(packet.src_ip_address, _ipv4_addr, 4);
    memcpy(packet.dest_ip_address, dest_ipv4_addr, 4);
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
    const uint8_t* dest_hw_addr = MAC_BROADCAST;

    // print_bytes("Packet: ", &packet, packet_length);
    
    // ---------- Make frame (Layer 2)! ---------- 
    return inet_write_frame(dest_hw_addr, FRAME_IPV4, &packet, packet_length, socket);
}

int inet_resolve_ip_address(const uint8_t* ipv4_addr, uint8_t* hw_addr) {
    memcpy(hw_addr, IPV4_BROADCAST, 4); // TODO: ONCE ARP IS DONE
    return INET_SUCCESS;
}
int inet_resolve_hw_address(const uint8_t* hw_addr, uint8_t* ipv4_addr) {
    memcpy(ipv4_addr, MAC_BROADCAST, 6); // TODO: ONCE ARP IS DONE
    return INET_SUCCESS;
}

/**********************************************************
 * Private interface
 */

int ipv4_check_header(const ipv4_t* packet, uint16_t header_bytes) {

    // Make sure it is IPV4!!!!!
    if (packet->version != 4) {
        return INET_NOT_IPV4;
    }

    if (packet->header_length_words != IPV4_PACKET_HEADER_WORDS) {
        trace("INET_IPV4_UNSUPPORTED_HEADER_LEN: %d, expected %d\n", packet->header_length_words, IPV4_PACKET_HEADER_WORDS);
        return INET_IPV4_UNSUPPORTED_HEADER_LEN;
    }

    if (header_bytes < ICMP_HEADER_BYTES || header_bytes > IPV4_MAX_SIZE) {
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
            err = inet_icmp_handler(packet->data, packet->src_ip_address, packet_bytes - IPV4_PACKET_HEADER_BYTES);
            return err;
        default:
            return INET_IPV4_UNSUPPORTED_PROTOCOL;
    }

    
    trace("IPv4 Not Handled quite yet\n");
    return INET_SUCCESS;
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

    if (_verbose_p) {
        print_bytes("Layer 3 ipv4: ", &_ipv4_rx, packet_bytes);
        print_bytes("Layer 3 ipv4: ", &_ipv4_rx, packet_bytes);
    }

    // 3. Check header
    err = ipv4_check_header(&_ipv4_rx, packet_bytes);
    if (err != INET_SUCCESS) {
        return err;
    }

    return ipv4_protocol_handler(&_ipv4_rx, packet_bytes);
}

#if 0

void inet_ipv4_handler(uint8_t *packet, uint16_t total_len) {

    uint8_t version, ihl;
    int err = inet_parse_header(ipv4, total_len, &version, &ihl);

    if (err != INET_SUCCESS) {
        trace("Error parsing IPv4 header: %d\n", err);
        return;
    }

    uint16_t packet_length = total_len - FRAME_HEADER_BYTES;
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

    ipv4_hdr_t *ip = (ipv4_hdr_t*)packet;

    if ((ip->version_ihl >> 4) != 4)
        return;

    uint8_t ihl = (ip->version_ihl & 0x0F) * 4;

    switch (ip->protocol)
    {
        case 1:
            icmp_process(packet + ihl, len - ihl);
            break;

        case 17:
            udp_process(packet + ihl, len - ihl);
            break;
    }

    // TODO: checksum
}



int inet_packet_handler(uint8_t* buffer, uint16_t* nbytes, uint16_t* protocol, uint8_t socket) {

    uint8_t packet_data[FRAME_MAX_PAYLOAD_SIZE];
    uint16_t read_bytes, ethertype;
    int err = inet_read_frame_data(packet_data, &read_bytes, &ethertype, socket);
    if (err != INET_SUCCESS) {
        // trace("Error handling frame: %d\n", err);
        return err;
    }

    // Check header based on ethertype
    //  switch (ethertype) {
    //     case FRAME_ARP:
    //         return inet_arp_handler(packet_data, read_bytes, socket);
    //     case FRAME_IPV4:
    //         return inet_ipv4_handler(packet_data, read_bytes, socket);
    //     default:
    //         // trace("Unsupported ethertype: %x\n", ethertype);
    //         return INET_FRAME_UNSUPPORTED_ETHERTYPE; // Don't handle this protocol
    // }

    // 
    return INET_SUCCESS;
}


#endif

/**********************************************************
 * Write!
 */
