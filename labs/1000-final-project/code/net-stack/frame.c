// LAYER 2: Ethernet Frame Handling with network interface (W5500 driver)

#include "inet.h"
#include "netif/w5500.h"
#include "../crc-16.h"
#include "../endian.h"

static const w5500_t* global_nic = NULL;

/**********************************************************
 * Setup
 */

void inet_nic_init(const w5500_t* nic) { global_nic = nic; }

/**********************************************************
 * Read!
 */

int inet_read_frame(frame_t* frame, uint16_t* nbytes, uint8_t socket) {
    if (global_nic == NULL) return INET_NIC_NOT_INITIALIZED; // NIC not initialized
    

    // Read frame
    *nbytes = w5500_read_rx_bytes(global_nic, frame, socket);
    
    if (*nbytes == 0) {
        w5500_fast_flush_rx(global_nic, socket);
        return INET_NO_DATA_READ;  // No data read
    }

    swapEndian16(&frame->ethertype); // Swap ethertype since it is sent big endian

    // Verify MAC address (multicast or unicast to us), and still returns so we can peek at the packet
    if (!((frame->dest_hw_addr[0] & 0x01) || memcmp(&frame->dest_hw_addr[0], global_nic->hw_addr, 6) == 0)) {
        return INET_NOT_FOR_US;  // Not addressed to us
    }

    return INET_SUCCESS;  // Valid frame
}

/**********************************************************
 * Write!
 */

uint16_t inet_send_ping(const uint8_t* dest_ipv4_addr, const void* data, uint16_t nbytes, uint8_t socket) {

    uint16_t icmp_length = nbytes + ICMP_HEADER_BYTES;

    icmp_echo_t icmp;
    icmp.type = ICMP_ECHO_MSG;
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
    memcpy(packet.src_ip_address, global_nic->ipv4_addr, 4);
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
    const uint8_t* dest_hw_addr = ETH_BROADCAST;

    // print_bytes("Packet: ", &packet, packet_length);
    
    // ---------- Make frame! ---------- 
    return inet_write_frame(dest_hw_addr, FRAME_IPV4, &packet, packet_length, socket);
}

uint16_t inet_write_frame(const uint8_t* dest_hw_addr, uint16_t ethertype, void* data, uint16_t nbytes, uint8_t socket) {
    if (global_nic == NULL) return INET_NIC_NOT_INITIALIZED;

    uint16_t frame_length = nbytes + FRAME_HEADER_BYTES;

    frame_t frame;
    memcpy(frame.dest_hw_addr, dest_hw_addr, 6);
    memcpy(frame.src_hw_addr, global_nic->hw_addr, 6);
    frame.ethertype = ethertype;
    memcpy(&frame.data, data, nbytes);

    swapEndian16(&frame.ethertype); // Swap ethertype since it is sent big endian

    // print_bytes("Frame: ", &frame, frame_length);
     
    return w5500_write_tx_bytes(global_nic, &frame, frame_length, socket);
}



/**********************************************************
 * Internal Helpers
 */

const w5500_t* inet_get_nic() { return global_nic; }
const uint8_t* inet_get_hw_addr() { return global_nic->hw_addr; }
const uint8_t* inet_get_ipv4_addr() { return global_nic->ipv4_addr; }
const uint8_t* inet_get_subnet_mask() { return global_nic->subnet_mask; }
const uint8_t* inet_get_gateway_addr() { return global_nic->gateway_addr; }

