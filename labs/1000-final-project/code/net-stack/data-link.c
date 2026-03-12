// LAYER 2: Ethernet Frame Handling with network interface (W5500 driver)

#include "inet.h"
#include "netif/w5500.h"
#include "data-link.h"

#include "arp.h"
#include "ipv4.h"
#include "icmp.h"

#include "../crc-16.h"
#include "../endian.h"
#include "../print-utilities.h"

static w5500_t* _nic = NULL;

static frame_t _frame_rx; // Buffer for reading frames

static int _verbose_p = 0;
static int _FULL_FRAME_verbose_p = 0;
static int _verbose_send_p = 0;

/**********************************************************
 * Setup
 */

// Level of verbosity
verbose_t inet_verbosity_init() {
    return (verbose_t){0};  // Sets all verbosities to 0
}

int inet_init(w5500_t* nic, const verbose_t* verbosity) {

    // Nullptr stuff to make sure no nullptr dereference
    verbose_t scrap = inet_verbosity_init();
    if (verbosity == NULL) {
        verbosity = &scrap;
    }

    if (verbosity->data_link || verbosity->all) {
        _verbose_p = 1;
        trace("Data link verbosity enabled\n");
    }

    if (verbosity->data_link_FULL_FRAME) {
        _FULL_FRAME_verbose_p = 1;
        trace("Data link MEGA FRAME verbosity enabled\n");    
    }  

    if (verbosity->data_link_send) {
        _verbose_send_p = 1;
        trace("Data link sending frame verbosity enabled\n");    
    }  

    // Layer 2
    _nic = nic;
    // inet_clear_arp_table(); this is done in arp init
    arp_init(verbosity);

    // Layer 3
    ipv4_init(verbosity);
    icmp_init(verbosity);

    return INET_SUCCESS;
}

/**********************************************************
 * Public Interface
 */

int inet_poll_frame(int flush_buffer) {
    if (_nic == NULL) return INET_NIC_NOT_INITIALIZED;

    // Read frame
    uint16_t read_bytes;
    int err = inet_read_frame(&_frame_rx, &read_bytes);
    if (err != INET_SUCCESS) {
        return err;
    }

    if (_verbose_p) {
        trace("Polled frame (%d bytes)\n", read_bytes);
    }
    if (_FULL_FRAME_verbose_p) {
        print_bytes("Frame bytes MEGA PRINT: ", &_frame_rx, read_bytes);
    }

    // 1. Verify MAC address (multicast or unicast to us), and still returns so we can peek at the packet
    int is_multicast = (_frame_rx.dest_hw_addr[0] & 0x01);
    int is_for_us = (memcmp(_frame_rx.dest_hw_addr, _nic->hw_addr, MAC_ADDR_LENGTH) == 0);

    if (!(is_multicast || is_for_us)) {
        trace("Frame not for us but for {%X:%X:%X:%X:%X:%X}\n",
            _frame_rx.dest_hw_addr[0], _frame_rx.dest_hw_addr[1],
            _frame_rx.dest_hw_addr[2], _frame_rx.dest_hw_addr[3],
            _frame_rx.dest_hw_addr[4], _frame_rx.dest_hw_addr[5]);
        return INET_MAC_NOT_FOR_US;  // Not addressed to us
    }

    // 2. Handle based on ethertype (for now just handle IPv4, but could add more handling here)
    err = handle_ethertype(&_frame_rx, read_bytes);
    if (err < INET_SUCCESS) {
        return err; // Ethertype not handled
    }

    // 3. Flush at end
    if (flush_buffer) {
        w5500_fast_flush_rx(_nic, INET_NIC_SOCKET);
    }

    return err;
}

int inet_send_frame(const uint8_t* dest_hw_addr, uint16_t ethertype, const void* data, uint16_t nbytes) {
    if (_nic == NULL) return INET_NIC_NOT_INITIALIZED;

    uint16_t frame_length = nbytes + FRAME_HEADER_BYTES;

    frame_t frame;
    memcpy(frame.dest_hw_addr, dest_hw_addr, MAC_ADDR_LENGTH);
    memcpy(frame.src_hw_addr, _nic->hw_addr, MAC_ADDR_LENGTH);
    frame.ethertype = ethertype;
    memcpy(frame.data, data, nbytes);

    swapEndian16(&frame.ethertype); // Swap ethertype since it is sent big endian

    if (_verbose_send_p || _FULL_FRAME_verbose_p) {
        trace("Sending frame, ethertype %x, dest MAC {%X:%X:%X:%X:%X:%X}, length %d\n", ethertype,
            dest_hw_addr[0], dest_hw_addr[1], dest_hw_addr[2],
            dest_hw_addr[3], dest_hw_addr[4], dest_hw_addr[5],
            frame_length);
        print_bytes("", &frame, frame_length);
    }
        
     
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
    // trace("Read frame from W5500: %d bytes\n", *nbytes);
    
    if (*nbytes == 0) {
        // trace("wiping the whole buffer aaa\n");
        w5500_fast_flush_rx(_nic, INET_NIC_SOCKET);
        return INET_NO_DATA_READ;  // No data read
    }

    swapEndian16(&frame->ethertype); // Swap ethertype since it is sent big endian

    return INET_SUCCESS;  // Valid frame
}

//  https://www.cavebear.com/archive/cavebear/Ethernet/type.html
// [Reference](https://datatracker.ietf.org/doc/html/rfc9542 ) according to [this site](https://www.iana.org/assignments/ieee-802-numbers/ieee-802-numbers.xhtml)
int handle_ethertype(frame_t* frame, uint16_t frame_nbytes) {

    uint16_t frame_data_length = frame_nbytes - FRAME_HEADER_BYTES;

    if (_FULL_FRAME_verbose_p)
        trace("Handling frame with ethertype %x, payload length %d\n",
            frame->ethertype, frame_data_length);

    // Not handling IEEE 802.3 frames
    if (frame->ethertype <= FRAME_IEEE802_3) { return INET_FRAME_802_3; }
    
    // Jump to protocol handler based on ethertype
    switch (frame->ethertype) {

        case FRAME_IPV4:
            if (_verbose_p)
                trace("Dispatching to IPv4 handler\n");
            return inet_ipv4_handler(frame->data, frame_data_length);
        case FRAME_ARP:
            if (_verbose_p)
                trace("Dispatching to ARP handler stripping padding from received %d bytes to get to %d bytes for protocol\n", ARP_MESSAGE_BYTES, frame_nbytes);
            // ARP message is 46 total bytes (see below) but minimum frame read length sent is 60 bytes (uses padding to get to that)
            // FRAME_HEADER_BYTES + ARP_MESSAGE_BYTES = 46
            // Therefore we strip the padding and stick to the protocol
            return inet_arp_handler(frame->data, ARP_MESSAGE_BYTES);
        default:
            if (_verbose_p)
                trace("Unsupported ethertype %x\n", frame->ethertype);
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

