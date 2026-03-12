
#ifndef __UDP_H__
#define __UDP_H__

#include <stdint.h>
#include "net-defs.h"

void udp_init(const verbose_t* verbosity);

// https://www.rfc-editor.org/rfc/rfc768.txt
int inet_udp_handler(const uint8_t* data, const uint8_t* src_ipv4_addr, uint16_t nbytes);

#endif