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

// ----- ERROR CODES! -----
enum {
    INET_SUCCESS = 0,
    INET_ERROR = -1,



    // Frame
    INET_MAC_FILTERED = -2,
    INET_NOT_IPV4 = -3,
    INET_MAC_NOT_FOR_US = -4,
    INET_NO_DATA_READ = -5,
    INET_NIC_NOT_INITIALIZED = -6,
    INET_FRAME_802_3 = -8,

    // IPV4
    INET_IPV4_UNSUPPORTED_VERS = -10,
    INET_IPV4_UNSUPPORTED_HEADER_LEN = -11,


    INET_FRAME_UNSUPPORTED_ETHERTYPE = -100,
    INET_IPV4_UNSUPPORTED_PROTOCOL = -101,
    INET_ICMP_UNSUPPORTED_TYPE = -102,
};

/**********************************************************
 * Setup
 */

int inet_init(w5500_t* nic, int verbose_p);
int inet_layer3_init(uint8_t* ipv4_addr, int verbose_p);

/**********************************************************
 * Layer 2
 */

// Entry point to stack. Starts at Layer 2
int inet_poll_frame(uint8_t socket, int flush_buffer); 

uint16_t inet_write_frame(const uint8_t* dest_hw_addr, uint16_t ethertype, void* data, uint16_t nbytes, uint8_t socket);
#define inet_write_broadcast_frame(nic, data, nbytes, socket) \
    inet_write_frame(nic, MAC_BROADCAST, data, nbytes, socket)

/**********************************************************
 * Layer 3
 */

int inet_resolve_ip_address(const uint8_t* ipv4_addr, uint8_t* hw_addr);
int inet_resolve_hw_address(const uint8_t* hw_addr, uint8_t* ipv4_addr);

uint16_t inet_write_ipv4_packet(const uint8_t* dest_ipv4_addr, uint8_t ipv4_protocol, const void* data, uint16_t nbytes, uint8_t socket);
#define inet_write_broadcast_ipv4_packet(nic, ipv4_protocol, data, nbytes, socket) \
    inet_write_ipv4_packet(nic, IPV4_BROADCAST, ipv4_protocol, MAC_BROADCAST, data, nbytes, socket)

/**********************************************************
 * Layer 3.5 Protocol Handlers
 */

uint16_t inet_send_ping(const uint8_t* dest_ipv4_addr, uint8_t ping_type, const void* data, uint16_t nbytes, uint8_t socket);

/**********************************************************
 * Internal Helpers
 */

const w5500_t* inet_get_nic();
const uint8_t* inet_get_hw_addr();
const uint8_t* inet_get_ipv4_addr();
const uint8_t* inet_get_subnet_mask();
const uint8_t* inet_get_gateway_addr();

#endif
