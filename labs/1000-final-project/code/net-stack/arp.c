// Layer 2 (high-ish)

#include "arp.h"

#include "data-link.h" // For setting mac
#include "network.h" // For setting ip

#include "../print-utilities.h"
#include "../endian.h"



int inet_arp_handler(const uint8_t* data, uint16_t arp_length) {
    int err;

    arp_packet_t* arp = (arp_packet_t*)data;

    // ---------- 1. Swap endian ----------
    swapEndian16(&arp->hardware_type);
    swapEndian16(&arp->protocol_type);
    swapEndian16(&arp->operation);

    // ---------- 2. Check packet ----------

    // Physical, RJ45 ethernet
    if (arp->hardware_type != ARP_HTYPE_ETHERNET) {return INET_ARP_BAD_HTYPE; }
    if (arp->hardware_len != MAC_ADDR_LENGTH) { return INET_ARP_INVALID_MAC_LEN; }

    // IPV4 addresses
    if (arp->protocol_type != FRAME_IPV4) { return INET_ARP_BAD_HTYPE; }
    if (arp->protocol_len != IPV4_ADDR_LENGTH) { return INET_ARP_INVALID_IP_LEN; }

    print_bytes("ARP PACKET:", (void*)data, ARP_MESSAGE_BYTES);
    print_as_string("ARP PACKET:", (void*)data, ARP_MESSAGE_BYTES);

    // ---------- 3. Add to table ----------

    // ---------- 4. Logic ----------

    // Makes sure this is MY ip
    if (memcmp(arp->dest_ipv4_addr, inet_get_ipv4_addr(), IPV4_ADDR_LENGTH) != 0) {
        return INET_ARP_NOT_FOR_US;
    }

    trace("OUR packet!\n");

    switch (arp->operation) {
        case ARP_REQUEST: // They sent request and are asking
            return inet_send_arp(arp->src_hw_addr, arp->src_ipv4_addr, ARP_REPLY);
        case ARP_REPLY: // Do nothing
            return INET_SUCCESS;
        default:
            return INET_ARP_INVALID_OP;
    }
}

// https://www.rfc-editor.org/rfc/rfc826.html
int inet_send_arp(const uint8_t* their_hw_addr, const uint8_t* their_ipv4_addr, uint8_t operation) {

    if (operation != ARP_REPLY && operation != ARP_REQUEST) {
        trace("Invalid ARP operation (%x)\n", operation);
        return INET_ARP_INVALID_OP;
    }

    // ---------- 1. Make and populate packet ----------
    arp_packet_t arp;
    arp.hardware_type = ARP_HTYPE_ETHERNET;
    arp.protocol_type = FRAME_IPV4;
    arp.hardware_len = MAC_ADDR_LENGTH;
    arp.protocol_len = IPV4_ADDR_LENGTH;
    arp.operation = operation;

    memcpy(arp.src_hw_addr, inet_get_hw_addr(), MAC_ADDR_LENGTH);
    memcpy(arp.src_ipv4_addr, inet_get_ipv4_addr(), IPV4_ADDR_LENGTH);
    memcpy(arp.dest_hw_addr, their_hw_addr, MAC_ADDR_LENGTH);
    memcpy(arp.dest_ipv4_addr, their_ipv4_addr, IPV4_ADDR_LENGTH);

    // ---------- 2. Swap endian ----------
    swapEndian16(&arp.hardware_type);
    swapEndian16(&arp.protocol_type);
    swapEndian16(&arp.operation);

    return inet_send_frame(their_hw_addr, FRAME_ARP, &arp, ARP_MESSAGE_BYTES);
}

