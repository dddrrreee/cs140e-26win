// LAYER 2: Ethernet Frame Handling with network interface (W5500 driver)

#include "inet.h"
#include "netif/w5500.h"
#include "data-link.h"
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

    // Layer 3
    int err = inet_layer3_init(nic->ipv4_addr, verbose_p);
    return err;
}

/**********************************************************
 * Public Interface
 */

int inet_poll_frame(uint8_t socket, int flush_buffer) {

    // Read frame
    uint16_t read_bytes;
    int err = inet_read_frame(&_frame_rx, &read_bytes, socket);
    if (err != INET_SUCCESS) {
        return err;
    }

    if (_verbose_p) {
        print_bytes("Polled frame bytes: ", &_frame_rx, read_bytes);
        print_bytes("Polled frame string: ", &_frame_rx, read_bytes);
    }

    // 1. Verify MAC address (multicast or unicast to us), and still returns so we can peek at the packet
    if (!((_frame_rx.dest_hw_addr[0] & 0x01) || memcmp(&_frame_rx.dest_hw_addr[0], _nic->hw_addr, 6) == 0)) {
        return INET_MAC_NOT_FOR_US;  // Not addressed to us
    }

    // 2. Handle based on ethertype (for now just handle IPv4, but could add more handling here)
    err = handle_ethertype(&_frame_rx, read_bytes);
    if (err != INET_SUCCESS) {
        return err; // Ethertype not handled
    }

    // 3. Flush at end
    if (flush_buffer) {
        w5500_fast_flush_rx(_nic, socket);
    }

    return INET_SUCCESS;
}

uint16_t inet_write_frame(const uint8_t* dest_hw_addr, uint16_t ethertype, void* data, uint16_t nbytes, uint8_t socket) {
    if (_nic == NULL) return INET_NIC_NOT_INITIALIZED;

    uint16_t frame_length = nbytes + FRAME_HEADER_BYTES;

    frame_t frame;
    memcpy(frame.dest_hw_addr, dest_hw_addr, 6);
    memcpy(frame.src_hw_addr, _nic->hw_addr, 6);
    frame.ethertype = ethertype;
    memcpy(&frame.data, data, nbytes);

    swapEndian16(&frame.ethertype); // Swap ethertype since it is sent big endian

    // print_bytes("Frame: ", &frame, frame_length);
     
    return w5500_write_tx_bytes(_nic, &frame, frame_length, socket);
}



/**********************************************************
 * Private Interface
 */

// Swaps ethertype bytes to be correct too since it is sent big endian on the wire, but our platform is little endian
int inet_read_frame(frame_t* frame, uint16_t* nbytes, uint8_t socket) {
    if (_nic == NULL) return INET_NIC_NOT_INITIALIZED; // NIC not initialized
    

    // Read frame
    *nbytes = w5500_read_rx_bytes(_nic, frame, socket);
    
    if (*nbytes == 0) {
        w5500_fast_flush_rx(_nic, socket);
        return INET_NO_DATA_READ;  // No data read
    }

    swapEndian16(&frame->ethertype); // Swap ethertype since it is sent big endian

    return INET_SUCCESS;  // Valid frame
}

#if 0
// int inet_read_frame_data(uint8_t* frame_payload, uint16_t* nbytes, uint16_t* ethertype, uint8_t socket) {

//     frame_t frame;
//     uint16_t read_bytes;
//     int err = inet_read_frame(&frame, &read_bytes, socket);
//     if (err != INET_SUCCESS) {
//         // memcpy(frame_payload, &frame, read_bytes);
//         return err;
//     }

//     // Ethertype handled by caller
//     *ethertype = frame.ethertype;
//     memcpy(frame_payload, &frame.data, read_bytes - FRAME_HEADER_BYTES);
//     *nbytes = read_bytes - FRAME_HEADER_BYTES;

//     if (*ethertype <= FRAME_IEEE802_3) {
//         return INET_FRAME_802_3; // Not handling IEEE 802.3 frames
//     }

//     // Checking size (this may already be handled in w5500 though) TODO: delegate/split

//     // Put into circular buffer for threads?

//     return INET_SUCCESS;
// }

#endif

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

        // case FRAME_ARP:
        //     trace("ARP not implemented yet!\n"); // TODO: implement ARP
        //     // not_reached();
        //     return INET_FRAME_UNSUPPORTED_ETHERTYPE;

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

