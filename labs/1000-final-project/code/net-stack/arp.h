
#ifndef __ARP_H__
#define __ARP_H__

#include <stdint.h>
#include "net-defs.h"

// https://www.rfc-editor.org/rfc/rfc826.html
int inet_arp_handler(const uint8_t* data, uint16_t );

#endif