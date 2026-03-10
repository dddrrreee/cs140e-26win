/*
LAYER 3: Packet handling! 

IP layer and all the packet handling logic
- ARP and IPv4

*/ 

#include "inet.h"
// #include "../crc-16.h"
// #include "../endian.h"


int inet_frame_handler(const frame_t* frame, uint16_t nbytes, uint8_t socket) {

    // Checking size (this may already be handled in w5500 though) TODO: delegate/split
    
    // Check ethertype and send to respective protocol handler
    switch (frame->ethertype) {
        case FRAME_ARP:
            panic("ARP not implemented yet!"); // TODO: implement ARP
            not_reached();
        case FRAME_IPV4:
            return inet_ipv4_handle_frame(frame, nbytes, socket);
        default:
            return INET_NOT_FOR_US; // Not for us since we don't handle this protocol
    }
}

int inet_resolve_ip_address(const uint8_t* ipv4_addr, uint8_t* hw_addr) {
    return INET_NOT_IMPLEMENTED;
}
int inet_resolve_hw_address(const uint8_t* hw_addr, uint8_t* ipv4_addr) {
    return INET_NOT_IMPLEMENTED;
}
