#ifndef __INET_H__
#define __INET_H__
#include "rpi.h"
#include "spi.h"        // hw spi implementation.
#include "circular.h"   // for the circular queue.
#include "src-loc.h"    // For some version of logging
#include "netif/w5500.h"
#include "net-defs.h"

typedef struct w5500 w5500_t;   // forward declaration

// TODO: make a verbose_p that enables for each layer based on bits


/**********************************************************
 * Setup
 */

int inet_init(w5500_t* nic, const verbose_t* verbosity); // This is in DATA-LINK.C
verbose_t inet_verbosity_init();
/**********************************************************
 * Layer 2
 */

// Entry point to stack. Starts at Layer 2
int inet_poll_frame(int flush_buffer); 

int inet_send_frame(const uint8_t* dest_hw_addr, uint16_t ethertype, const void* data, uint16_t nbytes);
#define inet_send_broadcast_frame(ethertype, data, nbytes) \
    inet_send_frame(MAC_BROADCAST, ethertype, data, nbytes)

// https://www.rfc-editor.org/rfc/rfc826.html
int inet_send_arp(const uint8_t* their_hw_addr, const uint8_t* their_ipv4_addr, uint8_t operation);

// If INET_SUCCESS (table not filled), `table_index` is set to index of ip_addr (key)
// int inet_find_arp_entry(const uint8_t* ipv4_addr, const uint8_t* hw_addr, uint32_t* table_index);

int inet_add_arp_entry(const uint8_t* their_ipv4_addr, const uint8_t* their_hw_addr);
int inet_invalidate_arp_entry(const uint8_t* ipv4_addr);
void inet_clear_arp_table();
// TODO: print ARP table

/**********************************************************
 * Layer 3
 */

int inet_resolve_ip_address(const uint8_t* ipv4_addr, uint8_t* hw_addr);
int inet_resolve_hw_address(const uint8_t* hw_addr, uint8_t* ipv4_addr);

int inet_send_ipv4_packet(const uint8_t* dest_ipv4_addr, uint8_t ipv4_protocol, const void* data, uint16_t nbytes);
#define inet_send_broadcast_ipv4_packet(nic, ipv4_protocol, data, nbytes) \
    inet_send_ipv4_packet(nic, IPV4_BROADCAST, ipv4_protocol, MAC_BROADCAST, data, nbytes)

/**********************************************************
 * Layer 3.5 Protocol Handlers
 */

int inet_send_ping(const uint8_t* dest_ipv4_addr, uint8_t ping_type, const void* data, uint16_t nbytes);

/**********************************************************
 * Layer 4!
 */

int inet_clear_port_handlers();

int inet_udp_add_port_handler(uint16_t port, udp_port_handler_t handler);

// `src_port` is OUR port
int inet_udp_send(uint16_t src_port, uint16_t dest_port, const uint8_t* dest_ip, const void* data, uint16_t data_len);
// uint16_t tcp_udp_recv(void* buffer, uint16_t buffer_len, uint8_t* src_ip, uint16_t* src_port);


/**********************************************************
 * Internal Helpers
 */

const w5500_t* inet_get_nic();
const uint8_t* inet_get_hw_addr();
const uint8_t* inet_get_ipv4_addr();
const uint8_t* inet_get_subnet_mask();
const uint8_t* inet_get_gateway_addr();

#endif
