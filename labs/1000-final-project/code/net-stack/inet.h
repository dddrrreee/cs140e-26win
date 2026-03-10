#ifndef __INET_H__
#define __INET_H__
#include "rpi.h"
#include "spi.h"        // hw spi implementation.
#include "circular.h"   // for the circular queue.
#include "src-loc.h"    // For some version of logging
#include "netif/w5500.h"
#include "net-defs.h"

typedef struct w5500 w5500_t;   // forward declaration


// ----- ERROR CODES! -----
enum {
    INET_SUCCESS = 0,
    INET_ERROR = -1,
    INET_MAC_FILTERED = -2,
    INET_NOT_IPV4 = -3,
    INET_NOT_FOR_US = -4,
    INET_NO_DATA_READ = -5,

    // INET_FRAME

    INET_NIC_NOT_INITIALIZED = -6,

    INET_NOT_IMPLEMENTED = -100,
};

/**********************************************************
 * Setup
 */

void inet_nic_init(const w5500_t* nic);

/**********************************************************
 * Layer 2 Write!
 */

uint16_t inet_send_ping(const uint8_t* dest_ipv4_addr, const void* data, uint16_t nbytes, uint8_t socket);

uint16_t inet_write_ipv4_packet(const uint8_t* dest_ipv4_addr, uint8_t ipv4_protocol, const void* data, uint16_t nbytes, uint8_t socket);
#define inet_write_broadcast_ipv4_packet(nic, ipv4_protocol, data, nbytes, socket) \
    inet_write_ipv4_packet(nic, IPV4_BROADCAST, ipv4_protocol, ETH_BROADCAST, data, nbytes, socket)


uint16_t inet_write_frame(const uint8_t* dest_hw_addr, uint16_t ethertype, void* data, uint16_t nbytes, uint8_t socket);
#define inet_write_broadcast_frame(nic, data, nbytes, socket) \
    inet_write_frame(nic, ETH_BROADCAST, data, nbytes, socket)

/**********************************************************
 * Layer 2 Read!
 */

int inet_read_frame(frame_t* frame, uint16_t* nbytes, uint8_t socket);

// Validates frame and sends to respective protocol handler. Returns error codes
int inet_frame_handler(const frame_t* frame, uint16_t nbytes, uint8_t socket);

/**********************************************************
 * Layer 3 Protocol Handlers
 */

/*[Reference](https://datatracker.ietf.org/doc/html/rfc9542 ) according to [this site](https://www.iana.org/assignments/ieee-802-numbers/ieee-802-numbers.xhtml)*/
int inet_arp_handle_frame(const frame_t* frame, uint16_t nbytes, uint8_t socket);
int inet_ipv4_handle_frame(const frame_t* frame, uint16_t nbytes, uint8_t socket);


int inet_resolve_ip_address(const uint8_t* ipv4_addr, uint8_t* hw_addr);
int inet_resolve_hw_address(const uint8_t* hw_addr, uint8_t* ipv4_addr);



/**********************************************************
 * Internal Helpers
 */

const w5500_t* inet_get_nic();
const uint8_t* inet_get_hw_addr();
const uint8_t* inet_get_ipv4_addr();
const uint8_t* inet_get_subnet_mask();
const uint8_t* inet_get_gateway_addr();

#endif
