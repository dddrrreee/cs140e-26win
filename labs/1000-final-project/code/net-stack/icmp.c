// Layer 3 (high-ish)

#include "inet.h"

#include "../crc-16.h"
#include "../print-utilities.h"

static int _verbose_p = 0;

void icmp_init(const verbose_t* verbosity) {
    if (verbosity->icmp || verbosity->all) {
        _verbose_p = 1;
        trace("ICMP layer verbosity enabled\n");
    }
}

int inet_icmp_handler(const uint8_t* data, const uint8_t* src_addr, uint16_t icmp_bytes) {
    
    // https://www.rfc-editor.org/rfc/rfc792
    uint8_t icmp_type = data[0];

    if (_verbose_p)
        trace("ICMP packet received from %d.%d.%d.%d, type=%d, bytes=%d\n",
                src_addr[0], src_addr[1], src_addr[2], src_addr[3], icmp_type, icmp_bytes);


    switch (icmp_type) {
        case ICMP_ECHO_REQUEST: // Send back to the source of the echo request
            if (_verbose_p)
                trace("Sending ping ECHO reply to {%d.%d.%d.%d}\n", src_addr[0], src_addr[1], src_addr[2], src_addr[3]);
            const char* msg = "THIS IS A REPLY";
            return inet_send_ping(src_addr, ICMP_ECHO_REPLY, msg, strlen(msg));

        case ICMP_ECHO_REPLY:
            if (_verbose_p)
                trace("Received ping REPLY from {%d.%d.%d.%d}\n",
                    src_addr[0], src_addr[1], src_addr[2], src_addr[3]);
            return INET_ICMP_PING_RECV;
    }

    if (_verbose_p)
        trace("INET_ICMP_UNSUPPORTED_TYPE %d received from {%d.%d.%d.%d}\n",
            icmp_type, src_addr[0], src_addr[1], src_addr[2], src_addr[3]);
    return INET_ICMP_UNSUPPORTED_TYPE;
}


int inet_send_ping(const uint8_t* dest_ipv4_addr, uint8_t ping_type, const void* data, uint16_t nbytes) {

    uint16_t icmp_length = nbytes + ICMP_HEADER_BYTES;

    icmp_echo_t icmp;
    icmp.type = ping_type;
    icmp.code = 0;
    icmp.checksum = 0; // Checksum to be filled
    icmp.identifier = N_A; // Not used in icmp echo
    icmp.seq_number = N_A; // Not used in icmp echo
    memcpy(icmp.data, data, nbytes);

    uint8_t* cksum_ptr = (uint8_t*)&icmp.checksum;
    uint16_t checksum = our_crc16(&icmp, icmp_length);
    *cksum_ptr = ( (checksum >> 8) & 0xFF);
    *(cksum_ptr + 1) = (checksum & 0xFF);

    if (_verbose_p)
        trace("Sending ping of type %d and length %d to {%d.%d.%d.%d}\n",
            ping_type, icmp_length,
            dest_ipv4_addr[0], dest_ipv4_addr[1],
            dest_ipv4_addr[2], dest_ipv4_addr[3]);

    int err = inet_send_ipv4_packet(dest_ipv4_addr, PROTOCOL_ICMP, &icmp, icmp_length);
    if (err > INET_SUCCESS)
        return INET_ICMP_PING_SENT;
    
    return err;
}

