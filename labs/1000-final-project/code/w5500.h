#ifndef __W5500_H__
#define __W5500_H__
#include "rpi.h"
#include "spi.h"        // hw spi implementation.
#include "circular.h"   // for the circular queue.
#include "src-loc.h"    // For some version of logging

#include "w5500-defs.h"

// configuration settigs for W5500.
typedef struct {
    uint32_t chip_select;      
    uint32_t clk_div;   // Default of 40

    uint8_t hw_addr[6];
    uint8_t ipv4_addr[4];
    uint8_t subnet_mask[4];
    uint8_t gateway_addr[4];

    
    uint8_t phy_mode;

    uint8_t sockets_enabled; // nth bit is for enabled
    

} w5500_conf_t;

typedef struct {
    uint8_t protocol;
    uint16_t port;

    uint8_t dest_hw_addr[6];
    uint8_t dest_ip_addr[4];
    uint16_t dest_ip_port;

} w5500_socket_t;

typedef struct {
    uint32_t start_usec, // idk exactly what this one is used for
             tot_sent_msgs,
             tot_sent_bytes,

             tot_retrans,
             tot_lost,

             tot_recv_msgs,
             tot_recv_bytes;
} w5500_stats_t;

typedef struct w5500 {
    spi_t spi;

    uint8_t hw_addr[6];
    uint8_t ipv4_addr[6];
    uint8_t subnet_mask[4];
    uint8_t gateway_addr[4];

    cq_t recvq; // Maybe need this?

    w5500_stats_t stats;
} w5500_t;

/**********************************************************
 * Setup
 */

// Must init respective socket too
void w5500_init(w5500_t* nic, w5500_conf_t* config);

// void w5500_init_socket(w5500_t* nic, w5500_sock_conf_t* config);

void w5500_socket_command(w5500_t* nic, uint8_t socket, uint8_t command);

/**********************************************************
 * Things
 */

// synchronize them with ARP?
// Link/speed/duplex status
// Interrupt status

/**********************************************************
 * Buffers
 */
typedef struct { // https://www.ietf.org/rfc/rfc768.txt
    uint8_t dest_hw_addr[6];
    uint8_t src_hw_addr[6];
    uint16_t ethertype; // Length. Going to make it MAX_FRAME_PAYLOAD_SIZE bytes or less
    uint8_t payload[MAX_FRAME_PAYLOAD_SIZE];
} frame_t;

// Returns -1 if invalid socket or not enough bytes in buffer
int w5500_write_tx_bytes(const w5500_t* nic, void* buffer, uint32_t nbytes, uint8_t socket);

// void w5500_write_frame(const w5500_t* nic, frame_t* frame) {

//     uint16_t frame_size = frame->length + FRAME_HEADER_BYTES;

//     frame_t buffer;
//     memcpy(&buffer, frame, frame_size);

//     // Swap bytes around in the length field to make it BIG ENDIAN
//     buffer.length = (frame->length >> 8) | (frame->length << 8);

//     w5500_putn(nic, W5500_BLK_SOCKET_0, W5500_Sn_REG_TX_RD0,
//         &buffer, frame->length + FRAME_HEADER_BYTES);
// }
/**********************************************************
 * Hardware routines that use SPI to read/write 
 * registers
 */

// read 8 bits of data from <reg>
uint8_t w5500_get8(const w5500_t* nic, uint8_t block, uint16_t reg);
// write 8 bits of data to <reg>
uint8_t w5500_put8(const w5500_t* nic, uint8_t block, uint16_t reg, uint8_t val);
// write 8-bit <v> to <reg> and then check make get8(reg) = <v>.
uint8_t w5500_put8_chk_helper(src_loc_t l, const w5500_t* nic, uint8_t block, uint16_t reg, uint8_t val);

#define w5500_put8_chk(nic, block, reg, val) \
    w5500_put8_chk_helper(SRC_LOC_MK(), nic, block, reg, val)

// read <nbytes> of data pointed to by <bytes> from <reg>
uint8_t w5500_getn(const w5500_t* nic, uint8_t block, uint16_t reg, void *bytes, uint32_t nbytes);
// write <nbytes> of data pointed to by <bytes> to <reg>
uint8_t w5500_putn(const w5500_t* nic, uint8_t block, uint16_t reg, void *bytes, uint32_t nbytes);
// write <nbytes> of data pointed to by <bytes> to <reg> and check
uint8_t w5500_putn_chk_helper(src_loc_t l, const w5500_t* nic, uint8_t block, uint16_t reg, void *bytes, uint32_t nbytes);

#define w5500_putn_chk(nic, block, reg, bytes, nbytes) \
    w5500_putn_chk_helper(SRC_LOC_MK(), nic, block, reg, bytes, nbytes)

/**********************************************************
 * other routines
 */

// Print IP, MAC, port, etc.


#endif
