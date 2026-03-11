// LAYER 2: Ethernet Frame Handling with network interface (W5500 driver)

#include "inet.h"
#include "netif/w5500.h"
#include "data-link.h"

#include "arp.h"
#include "network.h"

#include "../crc-16.h"
#include "../endian.h"
#include "../print-utilities.h"

static w5500_t* _nic = NULL;

static frame_t _frame_rx; // Buffer for reading frames

static int _verbose_p = 0;

/**********************************************************
 * Setup
 */

int inet_init(w5500_t* nic, int verbose_p) {

    _verbose_p = (verbose_p >> 2) & 1;
    // Layer 2
    _nic = nic;
    inet_clear_arp_table();

    // Layer 3
    int err = inet_layer3_init(verbose_p);
    return err;
}

/**********************************************************
 * Public Interface
 */

int inet_poll_frame(int flush_buffer) {

    // Read frame
    uint16_t read_bytes;
    int err = inet_read_frame(&_frame_rx, &read_bytes);
    if (err != INET_SUCCESS) {
        return err;
    }

    if (_verbose_p) {
        print_bytes("Polled frame bytes: ", &_frame_rx, read_bytes);
        print_bytes("Polled frame string: ", &_frame_rx, read_bytes);
    }

    // 1. Verify MAC address (multicast or unicast to us), and still returns so we can peek at the packet
    if (!((_frame_rx.dest_hw_addr[0] & 0x01) || memcmp(&_frame_rx.dest_hw_addr[0], _nic->hw_addr, MAC_ADDR_LENGTH) == 0)) {
        return INET_MAC_NOT_FOR_US;  // Not addressed to us
    }

    // 2. Handle based on ethertype (for now just handle IPv4, but could add more handling here)
    err = handle_ethertype(&_frame_rx, read_bytes);
    if (err != INET_SUCCESS) {
        return err; // Ethertype not handled
    }

    // 3. Flush at end
    if (flush_buffer) {
        w5500_fast_flush_rx(_nic, INET_NIC_SOCKET);
    }

    return INET_SUCCESS;
}

int inet_send_frame(const uint8_t* dest_hw_addr, uint16_t ethertype, const void* data, uint16_t nbytes) {
    if (_nic == NULL) return INET_NIC_NOT_INITIALIZED;

    uint16_t frame_length = nbytes + FRAME_HEADER_BYTES;

    frame_t frame;
    memcpy(frame.dest_hw_addr, dest_hw_addr, MAC_ADDR_LENGTH);
    memcpy(frame.src_hw_addr, _nic->hw_addr, MAC_ADDR_LENGTH);
    frame.ethertype = ethertype;
    memcpy(&frame.data, data, nbytes);

    swapEndian16(&frame.ethertype); // Swap ethertype since it is sent big endian

    // print_bytes("Frame: ", &frame, frame_length);
     
    uint16_t bytes_written = w5500_write_tx_bytes(_nic, &frame, frame_length, INET_NIC_SOCKET);
    if (bytes_written == 0)
        return INET_WRITE_FRAME_ERROR;

    return INET_SUCCESS;
}



/**********************************************************
 * Private Interface
 */

// Swaps ethertype bytes to be correct too since it is sent big endian on the wire, but our platform is little endian
int inet_read_frame(frame_t* frame, uint16_t* nbytes) {
    if (_nic == NULL) return INET_NIC_NOT_INITIALIZED; // NIC not initialized
    

    // Read frame
    *nbytes = w5500_read_rx_bytes(_nic, frame, INET_NIC_SOCKET);
    
    if (*nbytes == 0) {
        w5500_fast_flush_rx(_nic, INET_NIC_SOCKET);
        return INET_NO_DATA_READ;  // No data read
    }

    swapEndian16(&frame->ethertype); // Swap ethertype since it is sent big endian

    return INET_SUCCESS;  // Valid frame
}

//  https://www.cavebear.com/archive/cavebear/Ethernet/type.html
// [Reference](https://datatracker.ietf.org/doc/html/rfc9542 ) according to [this site](https://www.iana.org/assignments/ieee-802-numbers/ieee-802-numbers.xhtml)
int handle_ethertype(frame_t* frame, uint16_t frame_nbytes) {

    // Not handling IEEE 802.3 frames
    if (frame->ethertype <= FRAME_IEEE802_3) { return INET_FRAME_802_3; }
    
    // Jump to protocol handler based on ethertype
    switch (frame->ethertype) {

        case FRAME_IPV4:
            int err = inet_ipv4_handler(frame->data, frame_nbytes - FRAME_HEADER_BYTES);
            return err; // Handle in caller

        case FRAME_ARP:
            if (frame_nbytes - FRAME_HEADER_BYTES == ARP_MESSAGE_BYTES) {
                trace("Invalid ARP Packet length. Read %d. Expected %d",
                    frame_nbytes - FRAME_HEADER_BYTES, ARP_MESSAGE_BYTES);
                return INET_ARP_INVALID_MSG_LEN;
            }
            inet_arp_handler(frame->data);
            // not_reached();
            return INET_FRAME_UNSUPPORTED_ETHERTYPE;

        default:
            return INET_FRAME_UNSUPPORTED_ETHERTYPE; // Don't handle this protocol
    }

    return INET_SUCCESS;
}
/**********************************************************
 * Internal Helpers
 */

const w5500_t* inet_get_nic() { return _nic; }
const uint8_t* inet_get_hw_addr() { return _nic->hw_addr; }
const uint8_t* inet_get_ipv4_addr() { return _nic->ipv4_addr; }
const uint8_t* inet_get_subnet_mask() { return _nic->subnet_mask; }
const uint8_t* inet_get_gateway_addr() { return _nic->gateway_addr; }

