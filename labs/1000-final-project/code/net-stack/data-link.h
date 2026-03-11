
#ifndef __DATA_LINK_H__
#define __DATA_LINK_H__

#include <stdint.h>
#include "net-defs.h"

int inet_read_frame(frame_t* frame, uint16_t* nbytes);

int handle_ethertype(frame_t* frame, uint16_t frame_nbytes);

#endif