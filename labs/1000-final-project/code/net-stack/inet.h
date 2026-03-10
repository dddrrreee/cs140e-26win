#ifndef __INET_H__
#define __INET_H__
#include "rpi.h"
#include "spi.h"        // hw spi implementation.
#include "circular.h"   // for the circular queue.
#include "src-loc.h"    // For some version of logging

#include "net-defs.h"

typedef struct w5500 w5500_t;   // forward declaration

/**********************************************************
 * Setup
 */



void inet_nic_init(const w5500_t* nic);

/**********************************************************
 * Internet!
 */

uint16_t inet_send_ping(const uint8_t* dest_ipv4_addr, const void* data, uint16_t nbytes, uint8_t socket);

uint16_t inet_write_ipv4_packet(const uint8_t* dest_ipv4_addr, uint8_t ipv4_protocol, const void* data, uint16_t nbytes, uint8_t socket);
#define inet_write_broadcast_ipv4_packet(nic, ipv4_protocol, data, nbytes, socket) \
    inet_write_ipv4_packet(nic, IPV4_BROADCAST, ipv4_protocol, ETH_BROADCAST, data, nbytes, socket)


uint16_t inet_write_frame(const uint8_t* dest_hw_addr, uint16_t ethertype, void* data, uint16_t nbytes, uint8_t socket);
#define inet_write_broadcast_frame(nic, data, nbytes, socket) \
    inet_write_frame(nic, ETH_BROADCAST, data, nbytes, socket)

#endif
