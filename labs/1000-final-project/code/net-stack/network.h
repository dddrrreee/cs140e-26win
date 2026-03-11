
#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <stdint.h>
#include "inet.h"

int ipv4_check_header(const ipv4_t* packet, uint16_t header_bytes);

// [Reference](https://datatracker.ietf.org/doc/html/rfc9542 ) according to [this site](https://www.iana.org/assignments/ieee-802-numbers/ieee-802-numbers.xhtml)
int inet_ipv4_handler(const uint8_t* data, uint16_t packet_bytes);

int ipv4_protocol_handler(const ipv4_t* packet, uint16_t packet_bytes);


#endif