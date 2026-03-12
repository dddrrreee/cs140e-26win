/*
LAYER 3: IPV4
*/ 

#include "inet.h"

#include "../endian.h"
#include "../print-utilities.h"

static udp_t _udp_rx; // UDP buffer. will do one for each protocol perhaps

static int _verbose_p = 0;

//----------------------------------------------------------
//              Setup
//----------------------------------------------------------


void udp_init(const verbose_t* verbosity) {
    if (verbosity->udp || verbosity->all) {
        _verbose_p = 1;
        trace("UDP verbosity enabled\n");
    }
}

//----------------------------------------------------------
//              Send!
//----------------------------------------------------------


int inet_udp_send(uint16_t src_port, uint16_t dest_port, const uint8_t* dest_ip, const void* data, uint16_t data_len) {

    // 1. Just data checking I think
    if (data_len > UDP_MAX_SIZE) {
        if (_verbose_p)
            trace("Data too large (%d bytes) to fit into UDP_MAX_SIZE (%d)\n", data_len, UDP_MAX_SIZE);
        return INET_UDP_DATA_TOO_LONG;
    }

    // 2. Make UDP struct to send
    udp_t udp_tx;
    udp_tx.src_port = src_port;
    udp_tx.dest_port = dest_port;
    udp_tx.length = data_len;
    udp_tx.checksum = 0; // Not doing this lmaoooo
    memcpy(udp_tx.data, data, data_len);

    // 3. Flip endians (literally everything smh)
    swapEndian16(&udp_tx.dest_port);
    swapEndian16(&udp_tx.src_port);
    swapEndian16(&udp_tx.length);
    
    int err = inet_send_ipv4_packet(dest_ip, PROTOCOL_UDP, &udp_tx, data_len + UDP_HEADER_BYTES);
    if (err == INET_SUCCESS) {
        return INET_UDP_SENT;
    }

    return err;

}

//----------------------------------------------------------
//              Receive!
//----------------------------------------------------------

int inet_udp_handler(const uint8_t* data, const uint8_t* src_ipv4_addr, uint16_t nbytes) {
    int err;
    memcpy(&_udp_rx, data, nbytes);

    // 0. Checksum TODO???
    // uint16_t recv_cksum = ipv4->checksum;
    // swapEndian16(&ipv4->total_length);


    // 1. Flipping endians (literally everything smh)
    swapEndian16(&_udp_rx.dest_port);
    swapEndian16(&_udp_rx.src_port);
    swapEndian16(&_udp_rx.length);
    // swapEndian16(&_udp_rx.checksum); // Don't care about this lowkey

    // 2. Length checking
    if (_udp_rx.length > nbytes) {
        if (_verbose_p)
            trace("UDP claims udp_length %d bytes but only received %d bytes\n", _udp_rx.length, nbytes);
        return INET_UDP_LENGTH_MISMATCH;
    }
    uint16_t payload_len = _udp_rx.length - UDP_HEADER_BYTES;

    // 3. Route to correct ports
    uint16_t their_port = _udp_rx.src_port;
    uint16_t our_port = _udp_rx.dest_port;


    switch (our_port) {
        case 42069:
            print_bytes("UDP: ", data, nbytes);
            print_dec("UDP: ", data, nbytes);
            print_as_string("UDP: ", data, nbytes);
            return INET_UDP_RECEIVED;
        default:
            if (_verbose_p)
                trace("UDP unsupported port %d from {%d.%d.%d.%d:%d}\n", our_port,
                    src_ipv4_addr[0], src_ipv4_addr[1], src_ipv4_addr[2], src_ipv4_addr[3], their_port);
            return INET_UDP_UNSUPPORTED_PORT;
    }

    return INET_SUCCESS;
}


