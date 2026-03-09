#ifndef __CRC_16_H__
#define __CRC_16_H__

#include <stdint.h>

uint16_t our_crc16(const uint8_t* data, uint32_t nbytes) {

  uint32_t sum = 0;
  int i = 0;

  while (i + 1 < nbytes) {
    sum += (data[i] << 8) | data[i+1];
    i += 2;
  }
  if ( nbytes - i == 1 ) {
    sum += (data[i] << 8);
  }
  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  return (uint16_t)(~sum);
}

#endif