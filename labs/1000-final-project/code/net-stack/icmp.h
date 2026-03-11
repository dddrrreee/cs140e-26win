
#ifndef __ICMP_H__
#define __ICMP_H__

#include <stdint.h>
#include "net-defs.h"

// https://www.rfc-editor.org/rfc/rfc792
int inet_icmp_handler(const uint8_t* data, const uint8_t* src_addr, uint16_t icmp_bytes);

#endif