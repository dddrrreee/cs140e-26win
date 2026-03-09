#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#include <stdint.h>

static inline void swapNibbles8(void* val) {
    uint8_t* u8 = val;
    *u8 = ((*u8 & 0xF0) >> 4) | 
           ((*u8 & 0x0F) << 4);
}

static inline void swapEndian16(void* val) {
    uint16_t* u16 = val;
    *u16 = ((*u16 & 0xFF00) >> 8) | 
           ((*u16 & 0x00FF) << 8);
}

static inline void swapEndian32(void* val) {
    uint32_t* u32 = val;
    *u32 = ((*u32 & 0xFF000000) >> 24) | 
           ((*u32 & 0x00FF0000) >> 8) | 
           ((*u32 & 0x0000FF00) << 8) | 
           ((*u32 & 0x000000FF) << 24);
}

#endif