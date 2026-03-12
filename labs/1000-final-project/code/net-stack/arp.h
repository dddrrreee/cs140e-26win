
#ifndef __ARP_H__
#define __ARP_H__

#include <stdint.h>
#include "net-defs.h"

// TODO: make more fields and invalidate, etc.


void arp_init(const verbose_t* verbosity);

// https://www.rfc-editor.org/rfc/rfc826.html
int inet_arp_handler(const uint8_t* data, uint16_t nbytes);

/**
 * Finds index of arp entry. 
 * 
 * Results
 * - Entry is found and valid --> INET_SUCCESS and `table_index` is at the index
 * - Entry is found and invalid --> INET_ARP_FOUND_BUT_INVALID and `table_index` is at the index
 * - Entry is not found --> INET_ARP_NO_TABLE_ENTRY and `table_index` is at first non-valid index
 * - Table full --> INET_ARP_TABLE_FULL and `table_index` 0
 */
int find_arp_entry_by_ipv4(const uint8_t* ipv4_addr, uint32_t* table_index);
int find_arp_entry_by_mac(const uint8_t* hw_addr, uint32_t* table_index);

// Uses `trace_nofn`
void print_arp_entry(const uint8_t* ip_addr, const uint8_t* hw_addr, int newline);
void trace_arp_table(const char* msg);

#endif