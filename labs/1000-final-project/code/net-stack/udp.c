/*
LAYER 3: IPV4
*/ 

#include "inet.h"

#include "../endian.h"
#include "../print-utilities.h"

static udp_t _udp; // UDP buffer. will do one for each protocol perhaps

static int _verbose_p = 0;

static udp_port_t _port_handlers[UDP_PORT_TABLE_SIZE];


//----------------------------------------------------------
//              Setup
//----------------------------------------------------------


void udp_init(const verbose_t* verbosity) {
    if (verbosity->udp || verbosity->all) {
        _verbose_p = 1;
        trace("UDP verbosity enabled\n");
    }

    inet_clear_port_handlers();
}

int inet_clear_port_handlers() { 
    memset(_port_handlers, 0, sizeof(_port_handlers)); 
    if (_verbose_p)
        trace("Cleared UDP port table with %d entries\n", UDP_PORT_TABLE_SIZE);
    return INET_SUCCESS;
}

int inet_udp_add_port_handler(uint16_t port, udp_port_handler_t handler){

    for (uint32_t i = 0; i < UDP_PORT_TABLE_SIZE; i++) {
        if (_port_handlers[i].handler == NULL) {
            _port_handlers[i].port = port;
            _port_handlers[i].handler = handler;
            if (_verbose_p)
                trace("Added UDP Port %d handler with function at %x\n", port, handler);
            return INET_SUCCESS;
        }
    }

    return INET_UDP_PORT_TABLE_FULL;
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
    // memset(&_udp, 0, sizeof(_udp));
    static udp_t udp;
    udp.src_port = src_port;
    udp.dest_port = dest_port;
    udp.length = data_len;
    udp.checksum = 0; // Not doing this lmaoooo
    memcpy(udp.data, data, data_len);

    // 3. Flip endians (literally everything smh)
    swapEndian16(&udp.dest_port);
    swapEndian16(&udp.src_port);
    swapEndian16(&udp.length);
    
    int err = inet_send_ipv4_packet(dest_ip, PROTOCOL_UDP, &udp, data_len + UDP_HEADER_BYTES);
    if (err >= INET_SUCCESS) {
        return INET_UDP_SENT;
    }
    return err;
}

//----------------------------------------------------------
//              Receive!
//----------------------------------------------------------

int inet_udp_handler(const uint8_t* data, const uint8_t* src_ipv4_addr, uint16_t nbytes) {
    int err;
    memcpy(&_udp, data, nbytes);

    if (_verbose_p)
        trace("Received UDP packet of %d bytes\n", nbytes);

    // 0. Checksum TODO???
    // uint16_t recv_cksum = ipv4->checksum;
    // swapEndian16(&ipv4->total_length);


    // 1. Flipping endians (literally everything smh)
    swapEndian16(&_udp.dest_port);
    swapEndian16(&_udp.src_port);
    swapEndian16(&_udp.length);
    // swapEndian16(&_udp.checksum); // Don't care about this lowkey

    // 2. Length checking
    if (_udp.length > nbytes) {
        if (_verbose_p)
            trace("Packet claims udp_length is %d bytes but only received %d bytes\n", _udp.length, nbytes);
        return INET_UDP_LENGTH_MISMATCH;
    }
    uint16_t payload_len = _udp.length - UDP_HEADER_BYTES;

    // 3. Route to correct ports
    uint16_t their_port = _udp.src_port;
    uint16_t our_port = _udp.dest_port;

    for (uint32_t i = 0; i < UDP_PORT_TABLE_SIZE; i++) {
        if (_port_handlers[i].handler && _port_handlers[i].port == our_port) {
            if (_verbose_p)
                trace("Calling UDP handler for port %d {%d.%d.%d.%d:%d}\n", our_port,
                    src_ipv4_addr[0], src_ipv4_addr[1], src_ipv4_addr[2], src_ipv4_addr[3], their_port);
            _port_handlers[i].handler(src_ipv4_addr, their_port, our_port, _udp.data, payload_len);
            return INET_UDP_SEND_TO_HANDLER;
        }
    }

    if (_verbose_p)
        trace("UDP unsupported port %d from {%d.%d.%d.%d:%d}\n", our_port,
            src_ipv4_addr[0], src_ipv4_addr[1], src_ipv4_addr[2], src_ipv4_addr[3], their_port);
    return INET_UDP_UNSUPPORTED_PORT;
}


