#include "crc-16.h"

uint16_t our_crc16(const void* data, uint32_t nbytes)  {
    
    uint8_t* bytes = (uint8_t*)data;
    uint32_t sum = 0;
    int i = 0;

    while (i + 1 < nbytes) {
        sum += (bytes[i] << 8) | bytes[i+1];
        i += 2;
    }
    if ( nbytes - i == 1 ) {
        sum += (bytes[i] << 8);
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (uint16_t)(~sum);
}